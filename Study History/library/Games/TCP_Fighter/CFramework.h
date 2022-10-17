#pragma once

// ================================
// 게임 프레임워크 (게임매니져)
// 게임의 모든 흐름을 제어하는 클래스
// 싱글톤으로 전역에서 관리한다.
// ---
// 맴버 변수
// _frameSkip : 프레임스킵을 체크하여 Render를 스킵할지 결정한다.
// _objList : 게임에 존재하는 모든 게임오브젝트를 등록하는 리스트 등록하지 않으면 아무것도 못한다.
// _llFrameTotal : 게임로직중 시간이 필요한것의 타이머 대용(로직프레임)
// _iObjectIDCounter : 오브젝트 아이디 부여할 카운터
// ---
// 맴버 함수
// Init : 프레임워크 모든것을 초기화 (생성자에서 호출, 리셋할떄 호출)
// Update : 다형성을 이용해 모든 게임오브젝트를 업데이트
// Render : 다형성을 이용해 모든 게임오브젝트를 업데이트
// KeyProcess : 키보드 입력시 메시지를 발생시킴
// FrameSkip : 프레임 스킵을 판단 _frameSkip을 이용, 메인루프에서 판단
// GetRenderFPS : 렌더링 FPS를 얻어옴
// GetLogicFPS : 로직 FPS를 얻어옴
// 
// Instantiate : _objList에 넣어주기
// Destroy	: _objList에 뺄것을 선정
// Collision : 공격자의 히트박스와 겹치는 객체 찾기
// ================================


#include "framework.h"
#include "CFrameSkip.h"
#include "CList.h"
#include "CBaseObject.h"
#include "CPlayerObject.h"
#include "CEffectObject.h"
#include "CCameraObject.h"
#define I_FRAMEWORK CFramework::GetInstance()
#define INSTANTIATE(pObj)	I_FRAMEWORK->Instantiate((CBaseObject*)pObj)	// 프레임워크에 new한 오브젝트 등록후 반환
#define DESTROY(pObj)		I_FRAMEWORK->Destroy((CBaseObject*)pObj)		// 이 오브젝트는 이 프레임 맨 마지막에 소멸됨

class CFramework {
#pragma region Singleton 
	// 싱글톤
private:
	CFramework();	// 전체 사이즈와 컬러 비트를 받습니다.
	virtual ~CFramework();
public:
	// 전역 인스턴스를 얻어올 전역 함수
	static CFramework *GetInstance() {
		static CFramework _Instance;
		return &_Instance;
	};
#pragma endregion
private:
	CFrameSkip _frameSkip;	// 프레임스킵 판단
	CList< CBaseObject *> _objList; // 게임오브젝트 리스트
	INT64 _llFrameTotal;			// 타이머 대용, 로직 프레임 누적

	// 이전의 조작
	DWORD _dwOldInput;

	// 카메라
	CCameraObject *_opCamera;
	// 플레이어
	CPlayerObject *_opPlayer;	// 조작가능한 플레이어 오브젝트

public:
	void Init();		// 초기화
	void Update();		// 메인루프 : 게임로직 업데이트
	void Render();		// 메인루프 : 오브젝트 랜더링
	void KeyProcess();
	BOOL FrameSkip();	// 메인루프 : 딜레이와 프레임스킵 판단 (true : 랜더링을 실시, flase : 렌더링 스킵)
	int GetRenderFPS();
	int GetLogicFPS();

	void SetPlayer(CPlayerObject *opPlayer);

public: // 게임오브젝트 관련
	CBaseObject *Instantiate(CBaseObject *bObj); // 객체생성
	void Destroy(CBaseObject *bObj);	// 객체삭제
	CBaseObject *FindObject(int iID);


	
};

