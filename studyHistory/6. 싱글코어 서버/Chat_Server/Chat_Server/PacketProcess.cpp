#include "PacketProcess.h"
extern std::map<DWORD, CChatRoom *> g_rooms;
extern int g_RoomCnt;

// 패킷을 분해해서 처리
BOOL PacketProc(CPacket *pPacket, WORD wMsgType, CClient *pClient) {
	switch (wMsgType) {
	case df_REQ_LOGIN: // 로그인 요청
		return ReqLogin(pClient, pPacket);
		break;
	case df_REQ_ROOM_LIST: // 룸 리스트 요청
		return ReqRoomList(pClient, pPacket);
		break;
	case df_REQ_ROOM_CREATE: // 룸 생성 요청
		return ReqRoomCreate(pClient, pPacket);
		break;
	case df_REQ_ROOM_ENTER: // 룸 입장 요청
		return ReqRoomEnter(pClient, pPacket);
		break;
	case df_REQ_CHAT: // 체팅 요청
		return ReqChat(pClient, pPacket);
		break;
	case df_REQ_ROOM_LEAVE: // 룸떠나기 요청
		return ReqRoomLeave(pClient, pPacket);
		break;
	case df_REQ_STRESS_ECHO: // 스트레스 테스트
		return ReqStressEcho(pClient, pPacket);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BYTE MakeChecksum(CPacket *pPacket, WORD MsgType) {
	int size = pPacket->GetDataSize();
	BYTE *pPtr = (BYTE *) pPacket->GetBufferPtr();
	int iChecksum = MsgType;
	for (int i = 0; i < size; i++) {
		iChecksum += *pPtr;
		pPtr++;
	}

	return (BYTE) iChecksum % 256;
}

#pragma region Request

//------------------------------------------------------------
// 1 Req 로그인
//
//
// WCHAR[15]	: 닉네임 (유니코드)
//------------------------------------------------------------
BOOL ReqLogin(CClient *pClient, CPacket *pPacket) {
#ifdef PRINTSTEP
	wprintf_s(L"ReqLogin .. UID[%d]\n", pClient->_ID);
#endif // PRINTSTEP
	// TODO 중복 다시만들기
	WCHAR buffer[15] = {0};
	std::wstring username;


	pPacket->GetData((char *) buffer, sizeof(WCHAR) * 15);
	username = buffer;
	// 닉네임 중복체크
	if (IsUsername(username) == false) {
		// 중복이 아님
		pClient->_EnterRoomID = df_LOBBY;
		InsertUsername(username, pClient->_ID);
#ifdef PRINTSTEP
		wprintf_s(L"ReqLogin  OK [%ls]\n", pClient->_username.c_str());
#endif
		ResPacket_Login(pClient, df_RESULT_LOGIN_OK, pClient->_ID);
	} else {
#ifdef PRINTSTEP
		wprintf_s(L"ReqLogin  DNICK [%ls]\n", pClient->_username.c_str());
#endif
		ResPacket_Login(pClient, df_RESULT_LOGIN_DNICK, pClient->_ID);
	}
	return TRUE;
}

//------------------------------------------------------------
// 3 Req 대화방 리스트
//
//	None
//------------------------------------------------------------
BOOL ReqRoomList(CClient *pClient, CPacket *pPacket) {
#ifdef PRINTSTEP
	wprintf_s(L"ReqRoomList .. UID[%d] \n", pClient->_ID);
#endif // PRINTSTEP
	ResPacket_RoomList(pClient, &g_rooms);
	return TRUE;
}
//------------------------------------------------------------
// 5 Req 대화방 생성
//
// 2Byte : 방제목 Size			유니코드 문자 바이트 길이 (널 제외)
// Size  : 방제목 (유니코드)
//------------------------------------------------------------
BOOL ReqRoomCreate(CClient *pClient, CPacket *pPacket) {
#ifdef PRINTSTEP
	wprintf_s(L"ReqRoomCreate .. UID[%d] \n", pClient->_ID);
#endif // PRINTSTEP

	WCHAR buffer[256] = {0};
	std::wstring sRoomTitle;
	WORD wTitleSize;

	*pPacket >> wTitleSize;
	// 256 자이상 예외처리
	if (wTitleSize > 256) {
#ifdef PRINTSTEP
		wprintf_s(L"ReqRoomCreate .. OVER TitleSize 256 [%d] \n", wTitleSize);
#endif // PRINTSTEP
		ResPacket_RoomCreate(pClient, df_RESULT_ROOM_CREATE_ETC, NULL);
	}

	pPacket->GetData((char *) buffer, wTitleSize);

	sRoomTitle = buffer;
	if (IsRoomname(sRoomTitle) == TRUE) {
		// 중복된 방제
#ifdef PRINTSTEP
		wprintf_s(L"ReqRoomCreate ..  DNICK [%ls]\n", sRoomTitle.c_str());
#endif // PRINTSTEP
		ResPacket_RoomCreate(pClient, df_RESULT_ROOM_CREATE_DNICK, NULL);
		return TRUE;
	}

	// 방생성
	++g_RoomCnt;
	CChatRoom *pRoom = InsertRoom(g_RoomCnt++);
	if (pRoom == NULL) {
		CRASH();
	}
	InsertRoomname(sRoomTitle, pRoom->_dwRoomID);


	// 로깅
#ifdef PRINTSTEP
	wprintf_s(L"RoomCreate RID[%d] ROOM[%ls][%d]  TOTAL ROOM [%d]\n", pRoom->_dwRoomID, pRoom->_wsTitle.c_str(), (int) pRoom->_wsTitle.length(), (int) g_rooms.size());
#endif // PRINTSTEP

	ResPacket_RoomCreate(pClient, df_RESULT_ROOM_CREATE_OK, pRoom);
	return TRUE;
}
//------------------------------------------------------------
// 7 Req 대화방 입장
//
//	4Byte : 방 No
//------------------------------------------------------------
BOOL ReqRoomEnter(CClient *pClient, CPacket *pPacket) {
#ifdef PRINTSTEP
	wprintf_s(L"ReqRoomEnter .. UID[%d] \n", pClient->_ID);
#endif // PRINTSTEP

	DWORD dwRID;
	CChatRoom *pRoom;
	*pPacket >> dwRID;
#ifdef PRINTSTEP
	wprintf_s(L"RoomID [%d] \n", dwRID);
#endif // PRINTSTEP
	pRoom = FindRoom(dwRID);
	if (pRoom == NULL) {
		// 방이 없거나
#ifdef PRINTSTEP
		wprintf_s(L"//////ROOM NOT FIND \n");
#endif // PRINTSTEP
		ResPacket_RoomEnter(pClient, df_RESULT_ROOM_ENTER_NOT, NULL);
	} else {
#ifdef PRINTSTEP
		wprintf_s(L"ROOM  FIND \n");
#endif // PRINTSTEP

		if (RoomEnter(pClient, pRoom) == FALSE) {
			wprintf_s(L"////// ReqRoomEnter() : CAN NOT ENTER \n");
			return FALSE;
		}

		ResPacket_RoomEnter(pClient, df_RESULT_ROOM_ENTER_OK, pRoom);
	}
	return TRUE;
}
//------------------------------------------------------------
// 9 Req 채팅송신
//
// 2Byte : 메시지 Size
// Size  : 대화내용(유니코드)
//------------------------------------------------------------
BOOL ReqChat(CClient *pClient, CPacket *pPacket) {
#ifdef PRINTSTEP
	wprintf_s(L"ReqChat .. UID[%d] \n", pClient->_ID);
#endif // PRINTSTEP

	if (pClient->_EnterRoomID == df_LOBBY)return FALSE;
	WORD wMsgSize;
	WCHAR wsMsg[512] = {0};
	*pPacket >> wMsgSize;
	if (wMsgSize > 512) {
#ifdef PRINTSTEP
		wprintf_s(L"ReqRoomCreate .. OVER wMsgSize 512 [%d] \n", wMsgSize);
#endif // PRINTSTEP
		return FALSE;
	}
	pPacket->GetData((char *) wsMsg, wMsgSize);
#ifdef PRINTSTEP
	wprintf_s(L"//MSG [%ws] \n", wsMsg);
#endif // PRINTSTEP

	ResPacket_Chat(pClient, pClient->_ID, wMsgSize, wsMsg);


	return TRUE;
}
//------------------------------------------------------------
// 11 Req 방퇴장 
//
// None
//------------------------------------------------------------
BOOL ReqRoomLeave(CClient *pClient, CPacket *pPacket) {
#ifdef PRINTSTEP
	wprintf_s(L"ReqRoomLeave .. UID[%d] \n", pClient->_ID);
#endif // PRINTSTEP
	CChatRoom *pRoom = FindRoom(pClient->_EnterRoomID);
	if (pRoom == NULL) {
		wprintf_s(L"////// pRoom is NULL !! \n");
		return FALSE;
	}
	ResPacket_RoomLeave(pClient, pClient->_ID);
	RoomLeave(pClient, pRoom);

	if (pRoom->_userList.empty()) {
#ifdef PRINTSTEP
		wprintf_s(L"Room[%d] Delete \n", pRoom->_dwRoomID);
#endif // PRINTSTEP
		// 마지막 유저가 나갔으면 종료
		ResPacket_RoomDelete(pClient, pRoom->_dwRoomID);
		EraseRoom(pRoom->_dwRoomID);
	}

	return TRUE;
}

//------------------------------------------------------------
// 스트레스 테스트용 에코  	700 ~ 900 바이트 길이의 문자열
//
// {
//			WORD		Size
//			Size		문자열 (WCHAR 유니코드)
// }
//------------------------------------------------------------
BOOL ReqStressEcho(CClient *pClient, CPacket *pPacket) {

	WORD wMsgSize;
	WCHAR wsMsg[1000] = {0};
	*pPacket >> wMsgSize;
	pPacket->GetData((char *) wsMsg, wMsgSize);

	ResPacket_StressEcho(pClient, wMsgSize, wsMsg);


	return TRUE;
}
#pragma endregion






#pragma region Response
//------------------------------------------------------------
// 2 Res 로그인                              
// 
// 1Byte	: 결과 (1:OK / 2:중복닉네임 / 3:사용자초과 / 4:기타오류)
// 4Byte	: 사용자 NO
//------------------------------------------------------------
void ResPacket_Login(CClient *pClient, byte byResult, DWORD dwUID) {
#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_Login client[%d] RESULT[%hhx], UID[%d] \n", pClient->_ID, byResult, dwUID);
#endif // PRINTSTEP

	st_PACKET_HEADER header;
	CPacket packet;

	switch (byResult) {
	case df_RESULT_LOGIN_OK:	//OK
		MakePacket_Login(&header, &packet, byResult, dwUID);
		break;
	case df_RESULT_LOGIN_DNICK: // 중복닉
		MakePacket_Login(&header, &packet, byResult, 0);
		break;
	case df_RESULT_LOGIN_MAX: // 사용자 초과
		MakePacket_Login(&header, &packet, byResult, 0);
		break;
	default:	// 기타
		MakePacket_Login(&header, &packet, df_RESULT_LOGIN_ETC, 0);
		break;
	}
#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_Login Header[%hhx][%hhx][%hu][%hu] \n", header.byCode,header.byCheckSum,header.wMsgType,header.wPayloadSize);
#endif // PRINTSTEP
	SendUnicast(pClient, &header, &packet);
}

void MakePacket_Login(st_PACKET_HEADER *pHeader, CPacket *pPacket, byte byResult, DWORD dwUID) {
	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << byResult;
	*pPacket << dwUID;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byCheckSum = MakeChecksum(pPacket, df_RES_LOGIN);
	pHeader->wMsgType = df_RES_LOGIN;
	pHeader->wPayloadSize = pPacket->GetDataSize();
}
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
void ResPacket_RoomList(CClient *pClient, std::map<DWORD, CChatRoom *> *pRooms) {
#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_RoomList .. UID[%d] Roomcnt[%d]\n", pClient->_ID,(int)pRooms->size());
#endif // PRINTSTEP


	st_PACKET_HEADER header;
	CPacket packet;

	MakePacket_RoomList(&header, &packet, pRooms);


#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_RoomList Header[%hhx][%hhx][%hu][%hu] \n", header.byCode, header.byCheckSum, header.wMsgType, header.wPayloadSize);
#endif // PRINTSTEP


	SendUnicast(pClient, &header, &packet);
}

void MakePacket_RoomList(st_PACKET_HEADER *pHeader, CPacket *pPacket, std::map<DWORD, CChatRoom *> *pRooms) {
	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	WORD wRoomcnt =  (WORD) pRooms->size();
	WORD wTitleSize;
	BYTE byUserCnt;
	CChatRoom *pRoom;
	CClient *pClient;

	*pPacket << wRoomcnt;

	for (auto roIter = pRooms->begin(); roIter != pRooms->end(); ++roIter) {
		pRoom = roIter->second;
		// 4Byte : 방 No
		*pPacket << pRoom->_dwRoomID;
		// 2Byte : 방이름 byte size
		wTitleSize = (WORD)pRoom->_wsTitle.length() * sizeof(WCHAR);
		*pPacket << wTitleSize;
		// Size  : 방이름 (유니코드)
		pPacket->PutData((char *) pRoom->_wsTitle.c_str(), wTitleSize);

		// 1Byte : 참여인원	
		byUserCnt = pRoom->_userList.size();
		*pPacket << byUserCnt;

		for (auto clIter = pRoom->_userList.begin(); clIter != pRoom->_userList.end(); ++clIter) {
			pClient = FindClient(*clIter);
			if (pClient == NULL) {
				wprintf_s(L"////// MakePacket_RoomList() : CAN NOT FIND CLIENT [%d]!!!!", *clIter);
				continue;
			}
			pPacket->PutData((char *) pClient->_username.c_str(), dfNICK_MAX_LEN * sizeof(WCHAR));
		}
	}

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byCheckSum = MakeChecksum(pPacket, df_RES_ROOM_LIST);
	pHeader->wMsgType = df_RES_ROOM_LIST;
	pHeader->wPayloadSize = pPacket->GetDataSize();
}
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
void ResPacket_RoomCreate(CClient *pClient, BYTE byResult, CChatRoom *pRoom) {
	wprintf_s(L"ResPacket_RoomCreate .. UID[%d] Result[%hhx] RID[%d] \n", pClient->_ID, byResult, pRoom->_dwRoomID);

	st_PACKET_HEADER header;
	CPacket packet;

	// 1Byte : 결과 (1:OK / 2:방이름 중복 / 3:개수초과 / 4:기타오류)
	switch (byResult) {
	case df_RESULT_ROOM_CREATE_OK:
		MakePacket_RoomCreate(&header, &packet, byResult, pRoom);
		break;
	case df_RESULT_ROOM_CREATE_DNICK:
		MakePacket_RoomCreate(&header, &packet, byResult, NULL);
		break;
	case df_RESULT_ROOM_CREATE_MAX:
		MakePacket_RoomCreate(&header, &packet, byResult, NULL);
		break;
	default:
		MakePacket_RoomCreate(&header, &packet, df_RESULT_ROOM_CREATE_ETC, NULL);
		break;
	}



#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_RoomCreate Header[%hhx][%hhx][%hu][%hu] \n", header.byCode, header.byCheckSum, header.wMsgType, header.wPayloadSize);
#endif // PRINTSTEP

	if (byResult == df_RESULT_ROOM_CREATE_OK) {
		SendBroadcast(NULL, &header, &packet);
	} else {
		SendUnicast(pClient, &header, &packet);
	}
}

void MakePacket_RoomCreate(st_PACKET_HEADER *pHeader, CPacket *pPacket, byte byResult, CChatRoom *pRoom) {
	WORD wTitleSize;

	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	pPacket->PutData((char *) &byResult, sizeof(BYTE));
	if (byResult == df_RESULT_ROOM_CREATE_OK) {
		*pPacket << pRoom->_dwRoomID;
		wTitleSize = pRoom->_wsTitle.length() * sizeof(WCHAR);
		*pPacket << wTitleSize;
		pPacket->PutData((char *) pRoom->_wsTitle.c_str(), wTitleSize);
	}

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byCheckSum = MakeChecksum(pPacket, df_RES_ROOM_CREATE);
	pHeader->wMsgType = df_RES_ROOM_CREATE;
	pHeader->wPayloadSize = pPacket->GetDataSize();

}
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
void ResPacket_RoomEnter(CClient *pClient, byte byResult, CChatRoom *pRoom) {
#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_RoomEnter .. UID[%d] Result[%hhx] RID[%d] \n", pClient->_ID,byResult,pRoom->_dwRoomID);
#endif // PRINTSTEP

	st_PACKET_HEADER header;
	CPacket packet;

	// 1Byte : 결과 (1:OK / 2:방No 오류 / 3:인원초과 / 4:기타오류)
	switch (byResult) {
	case df_RESULT_ROOM_ENTER_OK:
		MakePacket_RoomEnter(&header, &packet, byResult, pRoom);
		break;
	case df_RESULT_ROOM_ENTER_NOT:
		MakePacket_RoomEnter(&header, &packet, byResult, NULL);
		break;
	case df_RESULT_ROOM_ENTER_MAX:
		MakePacket_RoomEnter(&header, &packet, byResult, 0);
		break;
	default:
		MakePacket_RoomEnter(&header, &packet, df_RESULT_ROOM_ENTER_ETC, 0);
		break;
	}


#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_RoomEnter Header[%hhx][%hhx][%hu][%hu] \n", header.byCode, header.byCheckSum, header.wMsgType, header.wPayloadSize);
#endif // PRINTSTEP
	SendUnicast(pClient, &header, &packet);

	// 다른사람에게도 보내기
	if (df_RESULT_ROOM_ENTER_OK) {
		ResPacket_UserEnter(pClient, pRoom, &pClient->_username, pClient->_ID);
	}

}

void MakePacket_RoomEnter(st_PACKET_HEADER *pHeader, CPacket *pPacket, byte byResult, CChatRoom *pRoom) {
	WORD wTitleSize;
	BYTE byUserCnt;
	CClient *pClient;

	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << byResult;
	if (pRoom != NULL) {
		// 4Byte : 방 No
		*pPacket << pRoom->_dwRoomID;
		// 2Byte : 방이름 byte size
		wTitleSize = pRoom->_wsTitle.length() * sizeof(WCHAR);
		*pPacket << wTitleSize;
		// Size  : 방이름 (유니코드)
		pPacket->PutData((char *) pRoom->_wsTitle.c_str(), wTitleSize);

		// 1Byte : 참여인원	
		byUserCnt = pRoom->_userList.size();
		*pPacket << byUserCnt;

		for (auto clIter = pRoom->_userList.begin(); clIter != pRoom->_userList.end(); ++clIter) {
			pClient = FindClient(*clIter);
			if (pClient == NULL) {
				wprintf_s(L"////// MakePacket_RoomEnter() : CAN NOT FIND CLIENT [%d]!!!!", *clIter);
				continue;
			}
			pPacket->PutData((char *) pClient->_username.c_str(), dfNICK_MAX_LEN * sizeof(WCHAR));
			*pPacket << pClient->_ID;
		}
	}

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byCheckSum = MakeChecksum(pPacket, df_RES_ROOM_ENTER);
	pHeader->wMsgType = df_RES_ROOM_ENTER;
	pHeader->wPayloadSize = pPacket->GetDataSize();
}
//------------------------------------------------------------
// 10 Res 채팅수신 (아무때나 올 수 있음)  (나에겐 오지 않음)
//
// 4Byte : 송신자 No
//
// 2Byte : 메시지 Size
// Size  : 대화내용(유니코드)
//------------------------------------------------------------
void ResPacket_Chat(CClient *pClient, DWORD dwSenderID, WORD wMsgSize, WCHAR *pMsg) {
#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_Chat .. UID[%d] dwSenderID[%d], wMsgSize[%hu], pMsg[%ls]\n", pClient->_ID, dwSenderID, wMsgSize, pMsg);
#endif // PRINTSTEP
	st_PACKET_HEADER header;
	CPacket packet;
	CChatRoom *pRoom = FindRoom(pClient->_EnterRoomID);
	if (pRoom == NULL) CRASH();
	MakePacket_Chat(&header, &packet, dwSenderID, wMsgSize, pMsg);


#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_Chat Header[%hhx][%hhx][%hu][%hu] \n", header.byCode, header.byCheckSum, header.wMsgType, header.wPayloadSize);
#endif // PRINTSTEP
	SendBroadcastRoom(pRoom, pClient, &header, &packet);
}

void MakePacket_Chat(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD dwSenderID, WORD wMsgSize, WCHAR *pMsg) {
	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
// 4Byte : 송신자 No
	*pPacket << dwSenderID;
	// 2Byte : 메시지 Size
	*pPacket << wMsgSize;
	// Size  : 대화내용(유니코드)
	pPacket->PutData((char *) pMsg, wMsgSize);

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byCheckSum = MakeChecksum(pPacket, df_RES_CHAT);
	pHeader->wMsgType = df_RES_CHAT;
	pHeader->wPayloadSize = pPacket->GetDataSize();
}
//------------------------------------------------------------
// 12 Res 방퇴장 (수시)
//
// 4Byte : 사용자 No
//------------------------------------------------------------
void ResPacket_RoomLeave(CClient *pClient, DWORD dwUID) {
#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_RoomLeave .. UID[%d] , dwUID[%d]\n", pClient->_ID, dwUID);
#endif // PRINTSTEP


	st_PACKET_HEADER header;
	CPacket packet;
	CChatRoom *pRoom = FindRoom(pClient->_EnterRoomID);
	if (pRoom == NULL) CRASH();

	MakePacket_RoomLeave(&header, &packet, dwUID);



#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_RoomLeave Header[%hhx][%hhx][%hu][%hu] \n", header.byCode, header.byCheckSum, header.wMsgType, header.wPayloadSize);
#endif // PRINTSTEP
	SendBroadcastRoom(pRoom, NULL, &header, &packet);
}

void MakePacket_RoomLeave(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD dwUID) {
	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << dwUID;
	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byCheckSum = MakeChecksum(pPacket, df_RES_ROOM_LEAVE);
	pHeader->wMsgType = df_RES_ROOM_LEAVE;
	pHeader->wPayloadSize = pPacket->GetDataSize();
}
//------------------------------------------------------------
// 13 Res 방삭제 (수시)
//
// 4Byte : 방 No
//------------------------------------------------------------
void ResPacket_RoomDelete(CClient *pClient, DWORD dwRID) {
#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_RoomDelete .. UID[%d] dwRID[%d] \n", pClient->_ID, dwRID);
#endif // PRINTSTEP


	st_PACKET_HEADER header;
	CPacket packet;

	MakePacket_RoomDelete(&header, &packet, dwRID);


#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_RoomDelete Header[%hhx][%hhx][%hu][%hu] \n", header.byCode, header.byCheckSum, header.wMsgType, header.wPayloadSize);
#endif // PRINTSTEP

	SendBroadcast(NULL, &header, &packet);
}

void MakePacket_RoomDelete(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD dwRID) {

	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << dwRID;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byCheckSum = MakeChecksum(pPacket, df_RES_ROOM_DELETE);
	pHeader->wMsgType = df_RES_ROOM_DELETE;
	pHeader->wPayloadSize = pPacket->GetDataSize();
}
//------------------------------------------------------------
// 14 Res 타 사용자 입장 (수시)
//
// WCHAR[15] : 닉네임(유니코드)
// 4Byte : 사용자 No
//------------------------------------------------------------
void ResPacket_UserEnter(CClient *pClient, CChatRoom *pRoom, std::wstring *pUsername, DWORD dwUID) {
#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_UserEnter .. UID[%d] RID[%d], username[%ls], dwUID[%d]\n", pClient->_ID, pRoom->_dwRoomID, pUsername->c_str(), dwUID);
#endif // PRINTSTEP
	st_PACKET_HEADER header;
	CPacket packet;

	MakePacket_UserEnter(&header, &packet, pUsername, dwUID);



#ifdef PRINTSTEP
	wprintf_s(L"ResPacket_UserEnter Header[%hhx][%hhx][%hu][%hu] \n", header.byCode, header.byCheckSum, header.wMsgType, header.wPayloadSize);
#endif // PRINTSTEP
	SendBroadcastRoom(pRoom, pClient, &header, &packet);
}

void MakePacket_UserEnter(st_PACKET_HEADER *pHeader, CPacket *pPacket, std::wstring *pUsername, DWORD dwUID) {

	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	pPacket->PutData((char *) pUsername->c_str(), dfNICK_MAX_LEN * sizeof(WCHAR));
	*pPacket << dwUID;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byCheckSum = MakeChecksum(pPacket, df_RES_USER_ENTER);
	pHeader->wMsgType = df_RES_USER_ENTER;
	pHeader->wPayloadSize = pPacket->GetDataSize();
}

//------------------------------------------------------------
// 스트레스 테스트용 에코응답
//
// {
//			WORD		Size
//			Size		문자열 (WCHAR 유니코드)
// }
//------------------------------------------------------------

void ResPacket_StressEcho(CClient *pClient, WORD wSize, WCHAR *wsStress) {

	st_PACKET_HEADER header;
	CPacket packet;


	MakePacket_StressEcho(&header, &packet, wSize, wsStress);



	SendUnicast(pClient, &header, &packet);

}
void MakePacket_StressEcho(st_PACKET_HEADER *pHeader, CPacket *pPacket, WORD wSize, WCHAR *wsStress) {

	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	// 2Byte : 메시지 Size
	*pPacket << wSize;
	// Size  : 대화내용(유니코드)
	pPacket->PutData((char *) wsStress, wSize);

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byCheckSum = MakeChecksum(pPacket, df_RES_STRESS_ECHO);
	pHeader->wMsgType = df_RES_STRESS_ECHO;
	pHeader->wPayloadSize = pPacket->GetDataSize();
}
#pragma endregion
