#include "NetworkCore.h"
#include "PacketProcess.h"
#include "CFramework.h"
// 패킷을 분해해서 처리
BOOL SC_PacketProc(CPacket *pPacket) {
	st_PACKET_HEADER *pHeader = (st_PACKET_HEADER *) pPacket->GetBufferPtr(); // 마샬링
	pPacket->MoveReadPos(sizeof(st_PACKET_HEADER));

	switch (pHeader->byType) {
	case dfPACKET_SC_CREATE_MY_CHARACTER:
		rpCREATE_MY_CHARACTER(pPacket);
		break;
	case dfPACKET_SC_CREATE_OTHER_CHARACTER:
		// 하나하나 넣기
		rpCREATE_OTHER_CHARACTER(pPacket);
		break;
	case dfPACKET_SC_DELETE_CHARACTER:
		rpDELETE_CHARACTER(pPacket);
		break;
	case dfPACKET_SC_MOVE_START:
		rpMOVE_START(pPacket);
		break;
	case dfPACKET_SC_MOVE_STOP:
		rpMOVE_STOP(pPacket);
		break;
	case dfPACKET_SC_ATTACK1:
		rpATTACK1(pPacket);
		break;
	case dfPACKET_SC_ATTACK2:
		rpATTACK2(pPacket);
		break;
	case dfPACKET_SC_ATTACK3:
		rpATTACK3(pPacket);
		break;
	case dfPACKET_SC_DAMAGE:
		rpDAMAGE(pPacket);
		break;
	case dfPACKET_SC_ECHO:
		rpECHO(pPacket);
	default:
		return FALSE;
	}
	return TRUE;
}

void rpCREATE_MY_CHARACTER(CPacket *pPacket) {
	//---------------------------------------------------------------
	// 클라이언트 자신의 캐릭터 할당		Server -> Client
	//
	// 서버에 접속시 최초로 받게되는 패킷으로 자신이 할당받은 ID 와
	// 자신의 최초 위치, HP 를 받게 된다. (처음에 한번 받게 됨)
	// 
	// 이 패킷을 받으면 자신의 ID,X,Y,HP 를 저장하고 캐릭터를 생성시켜야 한다.
	//
	//	4	-	ID
	//	1	-	Direction	(LL / RR)
	//	2	-	X
	//	2	-	Y
	//	1	-	HP
	//
	//---------------------------------------------------------------
	// 메시지 해석
	if (pPacket == NULL) return;
	DWORD ID;
	BYTE Dir;
	WORD X;
	WORD Y;
	BYTE HP;
	*pPacket >> ID >> Dir >> X >> Y >> HP;

	// 생성
	I_FRAMEWORK->SetPlayer( 
		(CPlayerObject *) INSTANTIATE(
		new CPlayerObject(
			ID, Dir, X, Y, HP, TRUE)));
}

void rpCREATE_OTHER_CHARACTER(CPacket *pPacket) {
	//---------------------------------------------------------------
	// 다른 클라이언트의 캐릭터 생성 패킷		Server -> Client
	//
	// 처음 서버에 접속시 이미 접속되어 있던 캐릭터들의 정보
	// 또는 게임중에 접속된 클라이언트들의 생성용 정보.
	//
	//
	//	4	-	ID
	//	1	-	Direction	(LL / RR)
	//	2	-	X
	//	2	-	Y
	//	1	-	HP
	//
	//---------------------------------------------------------------
	// 메시지 해석
	if (pPacket == NULL) return;
	DWORD ID;
	BYTE Dir;
	WORD X;
	WORD Y;
	BYTE HP;
	*pPacket >> ID >> Dir >> X >> Y >> HP;
	// 생성
	INSTANTIATE(new CPlayerObject(ID, Dir, X, Y, HP));
	wprintf_s(L"Create character ID [%d] Dir[%d] pos[%d, %d], HP[%d]\n", ID, Dir, X, Y, HP);

}

void rpDELETE_CHARACTER(CPacket *pPacket) {
	//---------------------------------------------------------------
	// 캐릭터 삭제 패킷						Server -> Client
	//
	// 캐릭터의 접속해제 또는 캐릭터가 죽었을때 전송됨.
	//
	//	4	-	ID
	//
	//---------------------------------------------------------------
	// 메시지 해석
	if (pPacket == NULL) return;
	DWORD ID;
	*pPacket >> ID;
	// 삭제
	DESTROY(I_FRAMEWORK->FindObject(ID));
	wprintf_s(L"delete character ID [%d]\n",ID);
}

void rpMOVE_START(CPacket *pPacket) {
	//---------------------------------------------------------------
	// 캐릭터 이동시작 패킷						Server -> Client
	//
	// 다른 유저의 캐릭터 이동시 본 패킷을 받는다.
	// 패킷 수신시 해당 캐릭터를 찾아 이동처리를 해주도록 한다.
	// 
	// 패킷 수신 시 해당 키가 계속해서 눌린것으로 생각하고
	// 해당 방향으로 계속 이동을 하고 있어야만 한다.
	//
	//	4	-	ID
	//	1	-	Direction	( 방향 디파인 값 8방향 )
	//	2	-	X
	//	2	-	Y
	//
	//---------------------------------------------------------------
	// 메시지 해석
	if (pPacket == NULL) return;
	DWORD ID;
	BYTE Dir;
	WORD X;
	WORD Y;
	*pPacket >> ID >> Dir >> X >> Y;

	// 처리
	CPlayerObject *opPlayer = (CPlayerObject *) I_FRAMEWORK->FindObject(ID);
	opPlayer->SetPosition(X, Y);
	opPlayer->ActionInput(Dir);
}

void rpMOVE_STOP( CPacket *pPacket) {
	//---------------------------------------------------------------
	// 캐릭터 이동중지 패킷						Server -> Client
	//
	// ID 에 해당하는 캐릭터가 이동을 멈춘것이므로 
	// 캐릭터를 찾아서 방향과, 좌표를 입력해주고 멈추도록 처리한다.
	//
	//	4	-	ID
	//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
	//	2	-	X
	//	2	-	Y
	//
	//---------------------------------------------------------------
	// 메시지 해석
	if (pPacket == NULL) return;
	DWORD ID;
	BYTE Dir;
	WORD X;
	WORD Y;
	*pPacket >> ID >> Dir >> X >> Y;

	// 처리
	CPlayerObject *opPlayer = (CPlayerObject *) I_FRAMEWORK->FindObject(ID);
	opPlayer->SetPosition(X, Y);
	//opPlayer->ActionInput(Dir);
	opPlayer->ActionInput(dfACTION_STAND);
}

void rpATTACK1( CPacket *pPacket) {
	//---------------------------------------------------------------
	// 캐릭터 공격 패킷							Server -> Client
	//
	// 패킷 수신시 해당 캐릭터를 찾아서 공격1번 동작으로 액션을 취해준다.
	// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
	//
	//	4	-	ID
	//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
	//	2	-	X
	//	2	-	Y
	//
	//---------------------------------------------------------------	
	// 메시지 해석
	if (pPacket == NULL) return;
	DWORD ID;
	BYTE Dir;
	WORD X;
	WORD Y;
	*pPacket >> ID >> Dir >> X >> Y;

	// 처리
	CPlayerObject *opPlayer = (CPlayerObject *) I_FRAMEWORK->FindObject(ID);
	opPlayer->SetPosition(X, Y);
	//opPlayer->ActionInput(Dir);
	opPlayer->ActionInput(dfACTION_ATTACK1);
}

void rpATTACK2( CPacket *pPacket) {
	//---------------------------------------------------------------
	// 캐릭터 공격 패킷							Server -> Client
	//
	// 패킷 수신시 해당 캐릭터를 찾아서 공격2번 동작으로 액션을 취해준다.
	// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
	//
	//	4	-	ID
	//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
	//	2	-	X
	//	2	-	Y
	//
	//---------------------------------------------------------------
	// 메시지 해석
	if (pPacket == NULL) return;
	DWORD ID;
	BYTE Dir;
	WORD X;
	WORD Y;
	*pPacket >> ID >> Dir >> X >> Y;

	// 처리
	CPlayerObject *opPlayer = (CPlayerObject *) I_FRAMEWORK->FindObject(ID);
	opPlayer->SetPosition(X, Y);
	//opPlayer->ActionInput(Dir);
	opPlayer->ActionInput(dfACTION_ATTACK2);
}

void rpATTACK3(CPacket *pPacket) {
	//---------------------------------------------------------------
	// 캐릭터 공격 패킷							Server -> Client
	//
	// 패킷 수신시 해당 캐릭터를 찾아서 공격3번 동작으로 액션을 취해준다.
	// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
	//
	//	4	-	ID
	//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
	//	2	-	X
	//	2	-	Y
	//
	//---------------------------------------------------------------
	// 메시지 해석
	if (pPacket == NULL) return;
	DWORD ID;
	BYTE Dir;
	WORD X;
	WORD Y;
	*pPacket >> ID >> Dir >> X >> Y;

	// 처리
	CPlayerObject *opPlayer = (CPlayerObject *) I_FRAMEWORK->FindObject(ID);
	opPlayer->SetPosition(X, Y);
	//opPlayer->ActionInput(Dir);
	opPlayer->ActionInput(dfACTION_ATTACK3);
}

void rpDAMAGE(CPacket *pPacket) {
	//---------------------------------------------------------------
	// 캐릭터 데미지 패킷							Server -> Client
	//
	// 공격에 맞은 캐릭터의 정보를 보냄.
	//
	//	4	-	AttackID	( 공격자 ID )
	//	4	-	DamageID	( 피해자 ID )
	//	1	-	DamageHP	( 피해자 HP )
	//
	//---------------------------------------------------------------
	// 메시지 해석
	if (pPacket == NULL) return;
	DWORD AtkID;
	DWORD DmgID;
	BYTE DmgHP;
	*pPacket >> AtkID >> DmgID >> DmgHP;
	wprintf_s(L"rpDAMAGE Attacker ID [%d] / Damage ID [%d], Damage HP[%d]\n", AtkID, DmgID, DmgHP);

	// 처리
	CPlayerObject *opAttack = (CPlayerObject *) I_FRAMEWORK->FindObject(AtkID);
	CPlayerObject *opDamage = (CPlayerObject *) I_FRAMEWORK->FindObject(DmgID);
	//if (opAttack == NULL) return;
	//if (opDamage == NULL) return;
	opDamage->SetHP(DmgHP);
	INSTANTIATE(new CEffectObject(opDamage->GetCurX(), opDamage->GetCurY() - 72));


}

void rpSYNK(CPacket *pPacket) {
	if (pPacket == NULL) return;
	DWORD ID;
	WORD X;
	WORD Y;

	*pPacket >> ID >> X >> Y;
	CPlayerObject *opPlayer = (CPlayerObject *) I_FRAMEWORK->FindObject(ID);
	if (opPlayer == NULL) return;
	opPlayer->SetPosition(X, Y);
}

void rpECHO(CPacket *pPacket) {
	CPacket packet;
	DWORD now = timeGetTime();
	int iMsgSize;
	iMsgSize = mpECHO(dfPACKET_CS_ECHO, &packet, now);
	SendPacket(packet.GetBufferPtr(), iMsgSize);
}

int mpMOVE_START(BYTE byTYPE, CPacket *pPacket, BYTE Dir, WORD X, WORD Y) {
	// 서버->클라 패킷은 만들면 안됨
	if (byTYPE == dfPACKET_SC_ATTACK1 ||
		byTYPE == dfPACKET_SC_ATTACK2 ||
		byTYPE == dfPACKET_SC_ATTACK3 ||
		byTYPE == dfPACKET_SC_CREATE_MY_CHARACTER ||
		byTYPE == dfPACKET_SC_CREATE_OTHER_CHARACTER ||
		byTYPE == dfPACKET_SC_DAMAGE ||
		byTYPE == dfPACKET_SC_DELETE_CHARACTER ||
		byTYPE == dfPACKET_SC_MOVE_START ||
		byTYPE == dfPACKET_SC_MOVE_STOP ||
		byTYPE == dfPACKET_SC_SYNC)
		return 0;
	int iLen = sizeof(Dir) + sizeof(X) + sizeof(Y);
	st_PACKET_HEADER stHeader;
	stHeader.byCode = 0x89;
	stHeader.bySize = iLen;
	stHeader.byType = byTYPE;

	*pPacket << stHeader.byCode << stHeader.bySize << stHeader.byType;
	*pPacket << Dir << X << Y;

	return iLen + sizeof(st_PACKET_HEADER);
}

int mpMOVE_STOP(BYTE byTYPE, CPacket *pPacket, BYTE Dir, WORD X, WORD Y) {
	// 서버->클라 패킷은 만들면 안됨
	if (byTYPE == dfPACKET_SC_ATTACK1 ||
		byTYPE == dfPACKET_SC_ATTACK2 ||
		byTYPE == dfPACKET_SC_ATTACK3 ||
		byTYPE == dfPACKET_SC_CREATE_MY_CHARACTER ||
		byTYPE == dfPACKET_SC_CREATE_OTHER_CHARACTER ||
		byTYPE == dfPACKET_SC_DAMAGE ||
		byTYPE == dfPACKET_SC_DELETE_CHARACTER ||
		byTYPE == dfPACKET_SC_MOVE_START ||
		byTYPE == dfPACKET_SC_MOVE_STOP ||
		byTYPE == dfPACKET_SC_SYNC)
		return 0;
	int iLen = sizeof(Dir) + sizeof(X) + sizeof(Y);
	st_PACKET_HEADER stHeader;
	stHeader.byCode = 0x89;
	stHeader.bySize = iLen;
	stHeader.byType = byTYPE;

	*pPacket << stHeader.byCode << stHeader.bySize << stHeader.byType;
	*pPacket << Dir << X << Y;

	return iLen + sizeof(st_PACKET_HEADER);
}

int mpATTACK1(BYTE byTYPE, CPacket *pPacket, BYTE Dir, WORD X, WORD Y) {
	// 서버->클라 패킷은 만들면 안됨
	if (byTYPE == dfPACKET_SC_ATTACK1 ||
		byTYPE == dfPACKET_SC_ATTACK2 ||
		byTYPE == dfPACKET_SC_ATTACK3 ||
		byTYPE == dfPACKET_SC_CREATE_MY_CHARACTER ||
		byTYPE == dfPACKET_SC_CREATE_OTHER_CHARACTER ||
		byTYPE == dfPACKET_SC_DAMAGE ||
		byTYPE == dfPACKET_SC_DELETE_CHARACTER ||
		byTYPE == dfPACKET_SC_MOVE_START ||
		byTYPE == dfPACKET_SC_MOVE_STOP ||
		byTYPE == dfPACKET_SC_SYNC)
		return 0;
	int iLen = sizeof(Dir) + sizeof(X) + sizeof(Y);
	st_PACKET_HEADER stHeader;
	stHeader.byCode = 0x89;
	stHeader.bySize = iLen;
	stHeader.byType = byTYPE;

	*pPacket << stHeader.byCode << stHeader.bySize << stHeader.byType;
	*pPacket << Dir << X << Y;

	return iLen + sizeof(st_PACKET_HEADER);
}

int mpATTACK2(BYTE byTYPE, CPacket *pPacket, BYTE Dir, WORD X, WORD Y) {
	// 서버->클라 패킷은 만들면 안됨
	if (byTYPE == dfPACKET_SC_ATTACK1 ||
		byTYPE == dfPACKET_SC_ATTACK2 ||
		byTYPE == dfPACKET_SC_ATTACK3 ||
		byTYPE == dfPACKET_SC_CREATE_MY_CHARACTER ||
		byTYPE == dfPACKET_SC_CREATE_OTHER_CHARACTER ||
		byTYPE == dfPACKET_SC_DAMAGE ||
		byTYPE == dfPACKET_SC_DELETE_CHARACTER ||
		byTYPE == dfPACKET_SC_MOVE_START ||
		byTYPE == dfPACKET_SC_MOVE_STOP ||
		byTYPE == dfPACKET_SC_SYNC)
		return 0;
	int iLen = sizeof(Dir) + sizeof(X) + sizeof(Y);
	st_PACKET_HEADER stHeader;
	stHeader.byCode = 0x89;
	stHeader.bySize = iLen;
	stHeader.byType = byTYPE;

	*pPacket << stHeader.byCode << stHeader.bySize << stHeader.byType;
	*pPacket << Dir << X << Y;

	return iLen + sizeof(st_PACKET_HEADER);
}

int mpATTACK3(BYTE byTYPE, CPacket *pPacket, BYTE Dir, WORD X, WORD Y) {
	// 서버->클라 패킷은 만들면 안됨
	if (byTYPE == dfPACKET_SC_ATTACK1 ||
		byTYPE == dfPACKET_SC_ATTACK2 ||
		byTYPE == dfPACKET_SC_ATTACK3 ||
		byTYPE == dfPACKET_SC_CREATE_MY_CHARACTER ||
		byTYPE == dfPACKET_SC_CREATE_OTHER_CHARACTER ||
		byTYPE == dfPACKET_SC_DAMAGE ||
		byTYPE == dfPACKET_SC_DELETE_CHARACTER ||
		byTYPE == dfPACKET_SC_MOVE_START ||
		byTYPE == dfPACKET_SC_MOVE_STOP ||
		byTYPE == dfPACKET_SC_SYNC)
		return 0;
	int iLen = sizeof(Dir) + sizeof(X) + sizeof(Y);
	st_PACKET_HEADER stHeader;
	stHeader.byCode = 0x89;
	stHeader.bySize = iLen;
	stHeader.byType = byTYPE;

	*pPacket << stHeader.byCode << stHeader.bySize << stHeader.byType;
	*pPacket << Dir << X << Y;

	return iLen + sizeof(st_PACKET_HEADER);
}

int mpSYNC(BYTE byTYPE, CPacket *pPacket, BYTE Dir, WORD X, WORD Y) {
	// 서버->클라 패킷은 만들면 안됨
	if (byTYPE == dfPACKET_SC_ATTACK1 ||
		byTYPE == dfPACKET_SC_ATTACK2 ||
		byTYPE == dfPACKET_SC_ATTACK3 ||
		byTYPE == dfPACKET_SC_CREATE_MY_CHARACTER ||
		byTYPE == dfPACKET_SC_CREATE_OTHER_CHARACTER ||
		byTYPE == dfPACKET_SC_DAMAGE ||
		byTYPE == dfPACKET_SC_DELETE_CHARACTER ||
		byTYPE == dfPACKET_SC_MOVE_START ||
		byTYPE == dfPACKET_SC_MOVE_STOP ||
		byTYPE == dfPACKET_SC_SYNC)
		return 0;
	int iLen = sizeof(Dir) + sizeof(X) + sizeof(Y);
	st_PACKET_HEADER stHeader;
	stHeader.byCode = 0x89;
	stHeader.bySize = iLen;
	stHeader.byType = byTYPE;

	*pPacket << stHeader.byCode << stHeader.bySize << stHeader.byType;
	*pPacket << Dir << X << Y;

	return iLen + sizeof(st_PACKET_HEADER);
}

int mpECHO(BYTE byTYPE, CPacket *pPacket, DWORD dwTime) {

	int iLen = sizeof(dwTime);
	st_PACKET_HEADER stHeader;
	stHeader.byCode = 0x89;
	stHeader.bySize = iLen;
	stHeader.byType = byTYPE;

	*pPacket << stHeader.byCode << stHeader.bySize << stHeader.byType;
	*pPacket << dwTime;

	return iLen + sizeof(st_PACKET_HEADER);
}

