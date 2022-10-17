#pragma once
// =========================================
// 모든 게임오브젝트의 부모 (루트)
// ---
// _bActivate		- 이 오브젝트가 살아있는지 false면 다음 프레임떄 소멸
// _dwActionInput	- 이 오브젝트가 입력을 받음 (서버 또는 키보드)
// _iObjectID		- 오브젝트의 고유 IP
// _eObjectType		- 이 오브젝트의 타입
// _iCurX, _iCurY	- 오브젝트의 (x, y)좌표
// _bEndFrame		- 오브젝트의 엑션스프라이트의 마지막인지 (에니메인션의 끝인지)
// _iDelayCount		- 스프라이트 사이의 딜레이를 젤 카운터
// _iFrameDelay		- 스프라이트 사이의 딜레이
// _iSpriteStart	- 에니메이션의 시작 스프라이트
// _iSpriteMax		- 에니메이션의 끝 스프라이트
// _iSpriteNow		- 현재 출력될 스프라이트
// ---
// 생성자 - 초기값 지정
// Update - (다형성) 이 겍체가 해야할 업데이트
// Render - (다형성) 이 겍체가 출력해야할 스프라이트
// Collision - (다형성) 이 겍체가 충돌시 해야할것 (피해자입장)
// Destroy - (다형성) 이 객체가 소멸해야할것을 한후 _bActivate = FALSE (안하면 삭제안됨)
// 
// NextFrame - 에니메이션 다음껄로!
// ActionInput - 이 겍체의 인풋 (서버, 키보드)
// SetSprite - 스프라이트셋트 설정
// =========================================



#include "framework.h"
class CBaseObject {
public :
	// 오브젝트 타입
	enum class e_OBJECTTYPE {
		PLAYER = 0,
		EFFECT,
		MAX
	};
protected:
	BOOL _bActivate;		// 객체 생존
	DWORD _dwActionInput;	// 객체가 입력받음
	DWORD _dwOldInput;	// 객체가 입력받음
	int _iObjectID;			// 오브젝트 식별 ID
	e_OBJECTTYPE _eObjectType;// 오브젝트 타입


	int _iCurX;				// x좌표
	int _iCurY;				// y촤표

	BOOL _bEndFrame;		// 에니메이션 끝
	int _iDelayCount;		// 에니메이션 동작 사이 딜레이 틱 카운터
	int _iFrameDelay;		// 에니메이션 동작 사이 딜레이

	int _iSpriteStart;		// 에니메이션 스프라이트 시작
	int _iSpriteMax;		// 에니메이션 스프라이트 끝
	int _iSpriteNow;		// 지금 출력중인 애니메이션 번호



public: // 생성,소멸
	CBaseObject();
	~CBaseObject();
public: // 가상함수
	virtual void Update();		// 오브젝트가 할 행동
	virtual void Render(BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch);	// 그리기
	virtual void Collision(CBaseObject* cAttacker); // 충돌 (내가 맞는 입장)
	virtual void Destroy(); // 삭제 _bActivate = false 해줘야함
	virtual void ActionInput(DWORD dwAction);	// 키보드 조작 또는 서버의 조작

public: // 할일
	void NextFrame();					// 에니메이션 다음프레임으로


public: // Setter
	void SetPosition(int iX, int iY);	// 좌표
	void SetSprite(int iSpriteStart, int iSpriteMax, int iFrameDelay); // 스프라이트

public: // Getter
	int GetCurX();
	int GetCurY();
	int GetObjectID();
	int GetObjectType();
	int GetSprite();
	BOOL IsActivate();
	BOOL IsEndFrame();
};

