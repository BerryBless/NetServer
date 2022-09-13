#include "pch.h"
#include "SMClient.h"

SMClient::SMClient() :CClient(false), _monitorServerID{ 0 }, _monitorServerIP{ 0 }, _monitorServerPort{0}{
	CClient::Start(1, 1, TRUE, 1);
}

SMClient::~SMClient() {
	CClient::Quit();
}

void SMClient::OnEnterServer(SESSION_ID sessionID) {
	_monitorServerID = sessionID;
}

void SMClient::OnLeaveServer(SESSION_ID sessionID) {
}

void SMClient::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
}

void SMClient::OnError(int errorcode, const WCHAR *) {
}

void SMClient::PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type) {
	ASSERT_CRASH(sessionID != _monitorServerID);

	switch (type) {
	default:
		break;
	}
}
void SMClient::PacketProcHeartBeat(Packet *pPacket) {
	SendPacket(_monitorServerID, pPacket);
}

void SMClient::MaketPakcetLogin(Packet *pPacket, int serverNo) {
	WORD type = PACKET_TYPE::en_PACKET_SS_MONITOR_LOGIN;
	*pPacket << type << serverNo;
}

void SMClient::MaketPakcetUpdate(Packet *pPacket, BYTE DataType, int DataValue, int TimeStamp) {
	WORD type = PACKET_TYPE::en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pPacket << type << DataType<<DataValue<<TimeStamp;
}



void SMClient::ConnectMonitorServer(const WCHAR *IP, USHORT port) {
	wsprintf(_monitorServerIP, L"%s", IP);
	_monitorServerPort = port;
	Connect(_monitorServerIP, _monitorServerPort);
}

void SMClient::SendMonitorPacket(BYTE DataType, int DataValue, int TimeStamp) {
	Packet *pPacket = Packet::AllocAddRef();
	MaketPakcetUpdate(pPacket, DataType, DataValue, TimeStamp);
	SendPacket(_monitorServerID, pPacket);
	pPacket->SubRef();
}
