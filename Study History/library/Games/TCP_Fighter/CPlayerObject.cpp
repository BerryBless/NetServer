#include "CPlayerObject.h"
#include "Managers.h"
#include "NetworkCore.h"
#include "PacketProcess.h" // 패킷 처리용

CPlayerObject::CPlayerObject(DWORD dwID, BYTE byDir, WORD woX, WORD woY, BYTE byHP, BOOL isPlayer) : CBaseObject() {
	_bPlayerCharacter = isPlayer;
	_iObjectID = dwID;
	_eObjectType = e_OBJECTTYPE::PLAYER;
	_eActionCur = e_PLAYERACTION::MAX;
	SetDirection((e_DIRECTION) byDir);
	SetPosition(woX, woY);
	SetHP(byHP);
	
}

CPlayerObject::~CPlayerObject() {
}
void CPlayerObject::Update() {
	NextFrame();
	ActionProc();
}

void CPlayerObject::Render(BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch) {
	// 출력
	CSpriteDib *spriteInst = I_SPRITEDIB;

	// 그림자 출력
	spriteInst->DrawSprite((int) e_SPRITE::eSHADOW, _iCurX, _iCurY, bypDest, iDestWidth, iDestHeight, iDestPitch);

	if (isPlayer()) {
		// 플레이어 캐릭터 출력
		spriteInst->DrawSprite(GetSprite(), _iCurX, _iCurY, bypDest, iDestWidth, iDestHeight, iDestPitch);
	}
	else {
		// 멀티플레이어는 빨갛게
		spriteInst->DrawSpriteRed(GetSprite(), _iCurX, _iCurY, bypDest, iDestWidth, iDestHeight, iDestPitch);
	}
	// HP바 출력
	spriteInst->DrawSprite((int) e_SPRITE::eGUAGE_HP, _iCurX -35, _iCurY +9, bypDest, iDestWidth, iDestHeight, iDestPitch, GetHP());
}

void CPlayerObject::ActionInput(DWORD dwAction) {
	_dwOldInput = _dwActionInput;
	_dwActionInput = dwAction;
	if (isPlayer()) {
		switch (_dwActionInput) {
		case dfACTION_STAND:
			SendMoveStopPacket();
			break;
		case dfACTION_MOVE_LL:
		case dfACTION_MOVE_LU:
		case dfACTION_MOVE_UU:
		case dfACTION_MOVE_RU:
		case dfACTION_MOVE_RR:
		case dfACTION_MOVE_RD:
		case dfACTION_MOVE_DD:
		case dfACTION_MOVE_LD:
			SendMoveStartPacket(_dwActionInput);
			break;
		case dfACTION_ATTACK1:
			SendAttackPacket(dfACTION_ATTACK1);
			break;
		case dfACTION_ATTACK2:
			SendAttackPacket(dfACTION_ATTACK2);
			break;
		case dfACTION_ATTACK3:
			SendAttackPacket(dfACTION_ATTACK3);
			break;
		default:
			break;
		}
	}

}

void CPlayerObject::SendMoveStartPacket(BYTE byDir) {
	CPacket clPacket;
	int iMsgSize;
	// 이동 패킷
	if (_dwOldInput != _dwActionInput) {
		iMsgSize = mpMOVE_START(dfPACKET_CS_MOVE_START, &clPacket, byDir, this->GetCurX(), this->GetCurY());
		SendPacket(clPacket.GetBufferPtr(), iMsgSize);
	}
}
void CPlayerObject::SendMoveStopPacket() {
	CPacket clPacket;
	int iMsgSize = 0;
	// 멈춤패킷
	if (this->GetCurAction() == CPlayerObject::e_PLAYERACTION::MOVE) {
		// 이동에만 관여해야함
		iMsgSize = mpMOVE_STOP(dfPACKET_CS_MOVE_STOP, &clPacket, this->GetDirection(), this->GetCurX(), this->GetCurY());
		SendPacket(clPacket.GetBufferPtr(), iMsgSize);
	}
}

void CPlayerObject::SendAttackPacket(BYTE byAttackType ) {
	CPacket clPacket;
	int iMsgSize = 0;
	// 멈춤패킷 (권장사항)
	SendMoveStopPacket();
	// 공격 3 패킷;
	iMsgSize = mpATTACK3(byAttackType, &clPacket, this->GetDirection(), this->GetCurX(), this->GetCurY());
	SendPacket(clPacket.GetBufferPtr(), iMsgSize);
}

void CPlayerObject::ActionProc() {
	// 플레이어가 공격중이면 에니메이션 변경 막기
	if (GetCurAction() == e_PLAYERACTION::ATTACK && _bEndFrame == FALSE) 
		return;
	
	e_DIRECTION eDir = e_DIRECTION::MAX;
	BOOL isMove = FALSE; // TRUE : 이동함
	int iDestX = GetCurX();	// 이동했을때 X좌표
	int iDestY = GetCurY(); // 이동했을때 Y좌표

	switch (_dwActionInput) {
	case dfACTION_MOVE_LL:
		// 왼쪽
		iDestX -= dfMOVE_PIXEL_HORIZONTAL;
		eDir = e_DIRECTION::LL;
		isMove = 1;
		break;
	case dfACTION_MOVE_LU:
		// 왼쪽 위
		iDestX -= dfMOVE_PIXEL_HORIZONTAL;
		iDestY -= dfMOVE_PIXEL_VERTICAL;
		eDir = e_DIRECTION::LL;
		isMove = 1;
		break;
	case dfACTION_MOVE_UU:
		// 위
		iDestY -= dfMOVE_PIXEL_VERTICAL;
		isMove = 1;
		break;
	case dfACTION_MOVE_RU:
		// 오른쪽 위
		iDestX += dfMOVE_PIXEL_HORIZONTAL;
		iDestY -= dfMOVE_PIXEL_VERTICAL;
		eDir = e_DIRECTION::RR;
		isMove = 1;
		break;
	case dfACTION_MOVE_RR:
		// 오른쪽
		iDestX += dfMOVE_PIXEL_HORIZONTAL;
		eDir = e_DIRECTION::RR;
		isMove = 1;
		break;
	case dfACTION_MOVE_RD:
		// 오른쪽 아래
		iDestX += dfMOVE_PIXEL_HORIZONTAL;
		iDestY += dfMOVE_PIXEL_VERTICAL;
		eDir = e_DIRECTION::RR;
		isMove = 1;
		break;
	case dfACTION_MOVE_DD:
		// 아래
		iDestY += dfMOVE_PIXEL_VERTICAL;
		isMove = 1;
		break;
	case dfACTION_MOVE_LD:
		// 왼쪽 아래
		iDestX -= dfMOVE_PIXEL_HORIZONTAL;
		iDestY += dfMOVE_PIXEL_VERTICAL;
		eDir = e_DIRECTION::LL;
		isMove = 1;
		break;



	case dfACTION_ATTACK1:
		// 공격1
		SetActionAttack1();
		// 키입력 초기화
		this->ActionInput(dfACTION_STAND);
		break;
	case dfACTION_ATTACK2:
		// 공격2
		SetActionAttack2();
		// 키입력 초기화
		this->ActionInput(dfACTION_STAND);
		break;
	case dfACTION_ATTACK3:
		// 공격3
		SetActionAttack3();
		// 키입력 초기화
		this->ActionInput(dfACTION_STAND);
		break;
	default:
		// 입력 없음
		SetActionStand();
		break;
	}

	if (isMove) {
		// 이동!
		// 이동제한 확인
		if (dfRANGE_MOVE_TOP < iDestY && iDestY < dfRANGE_MOVE_BOTTOM &&
			dfRANGE_MOVE_LEFT < iDestX && iDestX < dfRANGE_MOVE_RIGHT) {

			// 방향 전환 확인
			if (eDir != e_DIRECTION::MAX) {
				SetDirection(eDir);

				// 움직이는 도중 야매로 방향바꾸기
				// TODO 방향바꾸는 알고리즘 다시생각
				if (_eDirOld != _eDirCur)
					_eActionCur = e_PLAYERACTION::MAX;
			}
			// 진짜 이동
			SetPosition(iDestX, iDestY);
		}
		// 이동에니메이션 실행
		SetActionMove();
	}
	
}


#pragma region SetAction
//void CPlayerObject::InputActionProc(DWORD dwAction) {
//	_dwActionInput = dwAction;
//
//}
void CPlayerObject::SetActionAttack1() {
	_eActionOld = _eActionCur;
	_eActionCur = e_PLAYERACTION::ATTACK;

	if (_eDirCur == e_DIRECTION::LL||
		_eDirCur == e_DIRECTION::LD||
		_eDirCur == e_DIRECTION::LU) {
		SetSprite((int) e_SPRITE::ePLAYER_ATTACK1_L01, (int) e_SPRITE::ePLAYER_ATTACK1_L_MAX, dfDELAY_ATTACK1);
	} else if (_eDirCur == e_DIRECTION::RR ||
		_eDirCur == e_DIRECTION::RD ||
		_eDirCur == e_DIRECTION::RU) {
		SetSprite((int) e_SPRITE::ePLAYER_ATTACK1_R01, (int) e_SPRITE::ePLAYER_ATTACK1_R_MAX, dfDELAY_ATTACK1);
	}
}


void CPlayerObject::SetActionAttack2() {
	_eActionOld = _eActionCur;
	_eActionCur = e_PLAYERACTION::ATTACK;

	if (_eDirCur == e_DIRECTION::LL ||
		_eDirCur == e_DIRECTION::LD ||
		_eDirCur == e_DIRECTION::LU) {
		SetSprite((int) e_SPRITE::ePLAYER_ATTACK2_L01, (int) e_SPRITE::ePLAYER_ATTACK2_L_MAX, dfDELAY_ATTACK2);
	} else if (_eDirCur == e_DIRECTION::RR ||
		_eDirCur == e_DIRECTION::RD ||
		_eDirCur == e_DIRECTION::RU) {
		SetSprite((int) e_SPRITE::ePLAYER_ATTACK2_R01, (int) e_SPRITE::ePLAYER_ATTACK2_R_MAX, dfDELAY_ATTACK2);
	}
}

void CPlayerObject::SetActionAttack3() {
	_eActionOld = _eActionCur;
	_eActionCur = e_PLAYERACTION::ATTACK;

	if (_eDirCur == e_DIRECTION::LL ||
		_eDirCur == e_DIRECTION::LD ||
		_eDirCur == e_DIRECTION::LU) {
		SetSprite((int) e_SPRITE::ePLAYER_ATTACK3_L01, (int) e_SPRITE::ePLAYER_ATTACK3_L_MAX, dfDELAY_ATTACK3);
	} else if (_eDirCur == e_DIRECTION::RR ||
		_eDirCur == e_DIRECTION::RD ||
		_eDirCur == e_DIRECTION::RU) {
		SetSprite((int) e_SPRITE::ePLAYER_ATTACK3_R01, (int) e_SPRITE::ePLAYER_ATTACK3_R_MAX, dfDELAY_ATTACK3);
	}
}

void CPlayerObject::SetActionMove() {
	if (_eActionCur == e_PLAYERACTION::MOVE) {
		return;
	}
	_eActionOld = _eActionCur;
	_eActionCur = e_PLAYERACTION::MOVE;


	if (_eDirCur == e_DIRECTION::LL ||
		_eDirCur == e_DIRECTION::LD ||
		_eDirCur == e_DIRECTION::LU) {
		SetSprite((int) e_SPRITE::ePLATER_MOVE_L01, (int) e_SPRITE::ePLATER_MOVE_L_MAX, dfDELAY_MOVE);
	} else if (_eDirCur == e_DIRECTION::RR ||
		_eDirCur == e_DIRECTION::RD ||
		_eDirCur == e_DIRECTION::RU) {
		SetSprite((int) e_SPRITE::ePLATER_MOVE_R01, (int) e_SPRITE::ePLATER_MOVE_R_MAX, dfDELAY_MOVE);
	}
}

void CPlayerObject::SetActionStand() {
	if (_eActionCur == e_PLAYERACTION::STAND) return;

	_eActionOld = _eActionCur;
	_eActionCur = e_PLAYERACTION::STAND;


	// 방향에 다른 idle모션
	if (_eDirCur == e_DIRECTION::LL ||
		_eDirCur == e_DIRECTION::LD ||
		_eDirCur == e_DIRECTION::LU) {
		SetSprite((int) e_SPRITE::ePLAYER_STAND_L01, (int) e_SPRITE::ePLAYER_STAND_L_MAX, dfDELAY_STAND);
	} else if (_eDirCur == e_DIRECTION::RR ||
		_eDirCur == e_DIRECTION::RD||
		_eDirCur == e_DIRECTION::RU) {
		SetSprite((int) e_SPRITE::ePLAYER_STAND_R01, (int) e_SPRITE::ePLAYER_STAND_R_MAX, dfDELAY_STAND);
	}
}
#pragma endregion

void CPlayerObject::SetDirection(e_DIRECTION iDir) {
	_eDirOld = _eDirCur;
	_eDirCur = iDir;
}

void CPlayerObject::SetHP(int iHP) {
	//if (iHP < 0) return;
	_iHP = iHP;
}

int CPlayerObject::GetDirection() {
	return (int) _eDirCur;
}

int CPlayerObject::GetHP() {
	return _iHP;
}

BOOL CPlayerObject::isPlayer() {
	return _bPlayerCharacter;
}

CPlayerObject::e_PLAYERACTION CPlayerObject::GetCurAction() {
	return _eActionCur;
}

CPlayerObject::e_PLAYERACTION CPlayerObject::GetOldAction() {
	return _eActionOld;
}