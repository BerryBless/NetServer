#pragma once
#include <map>
#include "NetworkCore.h"
#include "Protocol.h"
#include "CPacket.h"
#include "CClient.h"
#include "CChatRoom.h"


//////////////////////////////////////////////////////////////////////////
// wMsgType타입에 따른 적절한 컨첸츠 처리함수를 호출한다
// 
// Parameters: CPacket *, WORD , CClient *
// Return: 성공 / 실패
//////////////////////////////////////////////////////////////////////////
BOOL PacketProc(CPacket *pPacket, WORD wMsgType, CClient *pClient); // 처리 성공시 TRUE

//////////////////////////////////////////////////////////////////////////
// 채크섬을 규칙에 따라 만든다
// checkSum - 각 MsgType, Payload 의 각 바이트 더하기 % 256
// 
// Parameters: CPacket *, WORD 
// Return: BYTE
//////////////////////////////////////////////////////////////////////////
BYTE MakeChecksum(CPacket *pPacket, WORD MsgType);

extern std::map<DWORD, CChatRoom *> g_rooms;
extern int g_RoomCnt;

#pragma region Request
//////////////////////////////////////////////////////////////////////////
// 직렬화 패킷에 따른 컨탠츠를 처리한다
// 
// Parameters: CClient *, CPacket *
// Return: 성공 / 실패
//////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------
// 1 Req 로그인
//
//
// WCHAR[15]	: 닉네임 (유니코드)
//------------------------------------------------------------
BOOL ReqLogin(CClient *pClient, CPacket *pPacket );
//------------------------------------------------------------
// 3 Req 대화방 리스트
//
//	None
//------------------------------------------------------------
BOOL ReqRoomList(CClient *pClient, CPacket *pPacket );
//------------------------------------------------------------
// 5 Req 대화방 생성
//
// 2Byte : 방제목 Size			유니코드 문자 바이트 길이 (널 제외)
// Size  : 방제목 (유니코드)
//------------------------------------------------------------
BOOL ReqRoomCreate(CClient *pClient, CPacket *pPacket );
//------------------------------------------------------------
// 7 Req 대화방 입장
//
//	4Byte : 방 No
//------------------------------------------------------------
BOOL ReqRoomEnter(CClient *pClient, CPacket *pPacket );
//------------------------------------------------------------
// 9 Req 채팅송신
//
// 2Byte : 메시지 Size
// Size  : 대화내용(유니코드)
//------------------------------------------------------------
BOOL ReqChat(CClient *pClient, CPacket *pPacket );
//------------------------------------------------------------
// 11 Req 방퇴장 
//
// None
//------------------------------------------------------------
BOOL ReqRoomLeave(CClient *pClient, CPacket *pPacket );

//------------------------------------------------------------
// 스트레스 테스트용 에코  	700 ~ 900 바이트 길이의 문자열
//
// {
//			WORD		Size
//			Size		문자열 (WCHAR 유니코드)
// }
//------------------------------------------------------------
BOOL ReqStressEcho(CClient *pClient, CPacket *pPacket);
#pragma endregion

#pragma region Response
//////////////////////////////////////////////////////////////////////////
// ResPacket.. (CClient *pClient ....)
// 클라에게 패킷을 보낸다!
// 
// Parameters: CClient *, ...
// Return: 없음
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//  MakePacket... ()
// 매개변수에 따른 직렬화 패킷을 만들고, 헤더를 만든다
// 
// Parameters: st_PACKET_HEADER *, CPacket *, ...
// Return: 없음
//////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------
// 2 Res 로그인                              
// 
// 1Byte	: 결과 (1:OK / 2:중복닉네임 / 3:사용자초과 / 4:기타오류)
// 4Byte	: 사용자 NO
//------------------------------------------------------------
void ResPacket_Login(CClient *pClient, byte byResult, DWORD dwUID);
void MakePacket_Login(st_PACKET_HEADER *pHeader, CPacket *pPacket, byte byResult, DWORD dwUID);
//------------------------------------------------------------
// 4 Res 대화방 리스트
//
//  2Byte	: 개수
//  {
//		4Byte : 방 No
//		2Byte : 방이름 byte size
//		Size  : 방이름 (유니코드)
//
//		1Byte : 참여인원		
//		{
//			WHCAR[15] : 닉네임
//		}
//	
//	}
//------------------------------------------------------------
void ResPacket_RoomList(CClient *pClient, std::map<DWORD, CChatRoom *> *pRooms);
void MakePacket_RoomList(st_PACKET_HEADER *pHeader, CPacket *pPacket, std::map<DWORD, CChatRoom *> *pRooms);

//------------------------------------------------------------
// 6 Res 대화방 생성 (수시로)
//
// 1Byte : 결과 (1:OK / 2:방이름 중복 / 3:개수초과 / 4:기타오류)
//
//
// 4Byte : 방 No
// 2Byte : 방제목 바이트 Size
// Size  : 방제목 (유니코드)
//------------------------------------------------------------
void ResPacket_RoomCreate(CClient *pClient, BYTE byResult,CChatRoom *pRoom);
void MakePacket_RoomCreate(st_PACKET_HEADER *pHeader, CPacket *pPacket, byte byResult, CChatRoom *pRoom);


//------------------------------------------------------------
// 8 Res 대화방 입장
//
// 1Byte : 결과 (1:OK / 2:방No 오류 / 3:인원초과 / 4:기타오류)
//
// OK 의 경우에만 다음 전송
//	{
//		4Byte : 방 No
//		2Byte : 방제목 Size
//		Size  : 방제목 (유니코드)
//
//		1Byte : 참가인원
//		{
//			WCHAR[15] : 닉네임(유니코드)
//			4Byte     : 사용자No
//		}
//	}
//------------------------------------------------------------
void ResPacket_RoomEnter(CClient *pClient, byte byResult, CChatRoom *pRoom);
void MakePacket_RoomEnter(st_PACKET_HEADER *pHeader, CPacket *pPacket, byte byResult, CChatRoom *pRoom);


//------------------------------------------------------------
// 10 Res 채팅수신 (아무때나 올 수 있음)  (나에겐 오지 않음)
//
// 4Byte : 송신자 No
//
// 2Byte : 메시지 Size
// Size  : 대화내용(유니코드)
//------------------------------------------------------------
void ResPacket_Chat(CClient *pClient, DWORD dwSenderID, WORD wMsgSize ,WCHAR *pMsg );
void MakePacket_Chat(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD dwSenderID, WORD wMsgSize, WCHAR *pMsg);


//------------------------------------------------------------
// 12 Res 방퇴장 (수시)
//
// 4Byte : 사용자 No
//------------------------------------------------------------
void ResPacket_RoomLeave(CClient *pClient, DWORD dwUID);
void MakePacket_RoomLeave(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD dwUID);


//------------------------------------------------------------
// 13 Res 방삭제 (수시)
//
// 4Byte : 방 No
//------------------------------------------------------------
void ResPacket_RoomDelete(CClient *pClient, DWORD dwRID);
void MakePacket_RoomDelete(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD dwRID);

//------------------------------------------------------------
// 14 Res 타 사용자 입장 (수시)
//
// WCHAR[15] : 닉네임(유니코드)
// 4Byte : 사용자 No
//------------------------------------------------------------
void ResPacket_UserEnter(CClient *pClient, CChatRoom *pRoom, std::wstring *pMsg, DWORD dwUID);
void MakePacket_UserEnter(st_PACKET_HEADER *pHeader, CPacket *pPacket, std::wstring *pMsg, DWORD dwUID);

//------------------------------------------------------------
// 스트레스 테스트용 에코응답
//
// {
//			WORD		Size
//			Size		문자열 (WCHAR 유니코드)
// }
//------------------------------------------------------------

void ResPacket_StressEcho(CClient *pClient, WORD wSize, WCHAR *wsStress);
void MakePacket_StressEcho(st_PACKET_HEADER *pHeader, CPacket *pPacket,WORD wSize, WCHAR *wsStress);
#pragma endregion
