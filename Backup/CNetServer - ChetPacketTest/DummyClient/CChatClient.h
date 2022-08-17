#pragma once
#include "CNetClient.h"
#include "ChatPlayer.h"

#define INVALID_PLAYER_SECTOR	51
#define SECTOR_X_SIZE			50
#define SECTOR_Y_SIZE			50
#define ID_MAX_LEN				20
#define ID_MAX_SIZE				40
#define NICK_NAME_MAX_LEN		20
#define NICK_NAME_MAX_SIZE		40
#define TOKEN_KEY_SIZE			64
#define MASSAGE_MAX_LEN			512
#define MASSAGE_MAX_SIZE		1024

class CChatClient : public CNetClient{

public:
	CChatClient();
	~CChatClient();

	
	void Login(ACCOUNT_NO ano, const WCHAR *ID, const WCHAR *Nick, const char *tokenkey);
	void TryMoveSector(WORD sx, WORD sy);
	void SendChatMessage(const WCHAR *msg);
	void Disconnect();
private:
	virtual void OnEnterJoinServer() ; //< 서버와의 연결 성공 후
	virtual void OnLeaveServer() ; //< 서버와의 연결이 끊어졌을 때
	virtual void OnRecv(CPacket *pPacket);
	virtual void OnSend(int sendsize) ;
	virtual void OnError(int errorcode, const WCHAR *);


	void PacketProc(CPacket *pPacket, WORD type);

	void PacketProcResponseLogin(CPacket *pPacket);
	void PacketProcResponseSectorMove(CPacket *pPacket);
	void PacketProcResponseMessage(CPacket *pPacket);

	void MakePacketRequestLogin(CPacket *pPacket, ACCOUNT_NO no, WCHAR *ID, WCHAR *nick, char* token);
	void MakePacketRequestSectorMove(CPacket *pPacket, ACCOUNT_NO no, WORD sectorX, WORD sectorY);
	void MakePacketRequestMessage(CPacket *pPacket, ACCOUNT_NO account_no, WORD msgLen, const WCHAR *message );

private:
	Player _player;
};

