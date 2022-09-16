#pragma once
#include "CClient.h"
#include "SerializingBuffer.h"
#include "CommonProtocol.h"
/// <summary>
/// 콘텐츠 서버 -> 모니터링 서버로 정보를 보내는 클라이언트
/// 프로토콜에 맞게 패킷을 만들고 보내기만 하면 되도록
/// </summary>

class SMClient : public CClient {
public:
	SMClient();
	~SMClient();

	virtual void OnEnterServer(SESSION_ID sessionID); //< 서버와의 연결 성공 후
	virtual void OnLeaveServer(SESSION_ID sessionID); //< 서버와의 연결이 끊어졌을 때
	virtual void OnRecv(SESSION_ID sessionID, Packet *pPacket);//< 패킷 수신 완료 후
	virtual void OnSend(SESSION_ID sessionID) {} //< 패킷 송신 완료 후
	virtual void OnError(int errorcode, const WCHAR *);//< 에러났을때 // TODO errorcode
	virtual void OnMonitoringPerSec() {};

	void PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type);
	void PacketProcHeartBeat(Packet *pPacket);
	void MaketPakcetLogin(Packet *pPacket, int ServerNo);
	void MaketPakcetUpdate(Packet *pPacket, BYTE serverNo, BYTE DataType, int DataValue, int TimeStamp);

public:
	bool ConnectMonitorServer(const WCHAR *IP, USHORT port, int serverNo);
	void SendMonitorPacket(BYTE serverNo, BYTE DataType, int DataValue, int TimeStamp);
private:
	WCHAR _monitorServerIP[20];
	USHORT _monitorServerPort;
	int _serverNo;
	SESSION_ID _monitorServerID;
};