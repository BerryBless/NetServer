#pragma once
#include "PacketDefine.h"
#include "CPacket.h"

BOOL SC_PacketProc(CPacket *pPacket); // 처리 성공시 TRUE

// 패킷 받기
// 생성
void rpCREATE_MY_CHARACTER(CPacket *pPacket);
void rpCREATE_OTHER_CHARACTER(CPacket *pPacket);
// 삭제
void rpDELETE_CHARACTER(CPacket *pPacket);
// 이동
void rpMOVE_START(CPacket *pPacket);
void rpMOVE_STOP(CPacket *pPacket);
// 공격
void rpATTACK1(CPacket *pPacket);
void rpATTACK2(CPacket *pPacket);
void rpATTACK3(CPacket *pPacket);
// 데미지
void rpDAMAGE(CPacket *pPacket);
// 동기화
void rpSYNK(CPacket *pPacket);
// 에코
void rpECHO(CPacket *pPacket);


// 패킷 만들어 보내기
//int MakePacket(BYTE byTYPE, CPacket *pPacket, BYTE byDir, WORD wX, WORD wY); //  패킷 만들기
int mpMOVE_START(BYTE byTYPE, CPacket *pPacket, BYTE byDir, WORD wX, WORD wY);
int mpMOVE_STOP(BYTE byTYPE, CPacket *pPacket, BYTE byDir, WORD wX, WORD wY);
int mpATTACK1(BYTE byTYPE, CPacket *pPacket, BYTE byDir, WORD wX, WORD wY);
int mpATTACK2(BYTE byTYPE, CPacket *pPacket, BYTE byDir, WORD wX, WORD wY);
int mpATTACK3(BYTE byTYPE, CPacket *pPacket, BYTE byDir, WORD wX, WORD wY);
int mpSYNC(BYTE byTYPE, CPacket *pPacket, BYTE byDir, WORD wX, WORD wY);
int mpECHO(BYTE byTYPE, CPacket *pPacket, DWORD dwTime);
