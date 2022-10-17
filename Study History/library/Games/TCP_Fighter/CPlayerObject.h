#pragma once
#include "CBaseObject.h"

// ==================================
// 플레이어 오브젝트
// ===
// CBaseObject <- CPlayerObject
// ---
// 멤버
// _bPlayerCharacter - 조작 가능한 캐릭터
// _iHP - 플레이어의 체력
// _eActionCur - "현재"액션
// _eActionOld - "이전"액션
// _eDirCur - "현재" 방향
// _eDirOld - "이전" 방향
// ---
// 생성자 - 모든값을 초기값지정
// Update - 에니메이션 다음프레임, 입력, 충돌체크 "요청"
// Render - _iSpriteNow의 스프라이트 출력, 그림자, _iHP에 따른 스프라이트 출력
// ActionProc - 입력(_dwActionInput)에 따른 처리
// SetAction~ - 해당 에니메이션으로 스프라이트셋을 변경
// Getter / Setter - 해당변수를 get/set
// ==================================


class CPlayerObject  : public CBaseObject{
public :
	// 플레이어의 액션
	enum class e_PLAYERACTION {
		STAND = 0,
		MOVE,
		ATTACK,
		MAX
	};

	// 플레이어의 방향
	enum class e_DIRECTION {
		LL = 0,
		LU,
		UU,
		RU,
		RR,
		RD,
		DD,
		LD,
		MAX
	};
	
private:
	BOOL _bPlayerCharacter;	// 조종가능한 플레이어 객체인지
	int _iHP;				// 플레이어 객체의 HP

	e_PLAYERACTION _eActionCur;	// 플레이어의 현재 액션
	e_PLAYERACTION _eActionOld; // 플레이어의 이전 액션
	e_DIRECTION _eDirCur;		// 플레이어의 현재 방향
	e_DIRECTION _eDirOld;		// 플레이어의 이전 방향

public:
	CPlayerObject(DWORD dwID, BYTE byDir, WORD woX, WORD woY, BYTE byHP, BOOL isPlayer = FALSE);
	~CPlayerObject();
public: // 가상함수
	void Update();
	void Render(BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch);
	void ActionInput(DWORD dwAction);
private:
	void ActionProc();		// Update내 실질적인 객체처리 함수
	void SendMoveStartPacket(BYTE byDir);
	void SendMoveStopPacket();
	void SendAttackPacket(BYTE byAttackType);
	// 에니메이션 변경
	void SetActionAttack1();
	void SetActionAttack2();
	void SetActionAttack3();
	void SetActionMove();
	void SetActionStand();
	

public: // Setter
	void SetDirection(e_DIRECTION iDir);
	void SetHP(int iHP);
public: // Getter
	int GetDirection();
	int GetHP();
	BOOL isPlayer();
	e_PLAYERACTION GetCurAction();
	e_PLAYERACTION GetOldAction();
};

