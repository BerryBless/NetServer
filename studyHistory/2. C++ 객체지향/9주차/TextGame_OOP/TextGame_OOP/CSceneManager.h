#pragma once
#include "IScene.h"

#define I_SCENEMANAGER CSceneManager::GetInstance()	// 인스턴스를 불러오는 매크로

// 현제 씬의 상태
enum class SceneState {
	Title = 0,	// 타이틀
	Game,		// 인게임
	End,		// 엔딩
};


// ============================================================================
//						CSceneManager
// ---------------------------------------------------------------------------
// 게임 씬을 관리하는 싱글톤 클래스
// 오브젝트 관리는 CFramework에서 함
// CSceneManager 는 게임 씬만 관리
// ---
// 맴버 변수
// _pScene		: 지금 프레임워크에서 실행중인 IScene을 가르키는 포인터
// _pResScene	: 다음씬 예약) 다음 씬을 미리 로드한 IScene을 가르키는 포인터
// _State		: 지금 어떤 신인지 상태
// _ResState	: 예약한 씬은 어떤 상태인가
// _inGame		: 게임이 진행중인 상태인가?
// _reset		: 게임이 리셋해야 하는지 리셋필요하면 true
// ---
// 맴버 함수
// Init()		: 이 씬의 초기상태로
// Update()		: 이 씬에서 매 프레임마다 해야할 일
// Render()		: 이 씬에서 매 프래임마다 화면에 출력할것
// Destroy()	: 이 씬이 끝날때 해야 할 일
// 
// Reset()		: 다음프레임 시작시 리셋 (Change()에서 리셋)!
// Load(scenename)	: 등록된 신중 scenename 과 일치하는것을 _pResScene에 보유해둠
// Change()		: 프레임의 첫 시작시 예약된 씬이 있으면 현재씬을 예약된 씬으로, 리셋을 해야하면 리셋을
// NowState()	: 현재 무슨 씬을 돌고있는지
// InGame()		: 게임이 진행중 이면 true 리턴
// ============================================================================
class CSceneManager {
#pragma region Singleton 
	// 싱글톤
private:
	CSceneManager();
	~CSceneManager();
public:
	// 전역 인스턴스를 얻어올 전역 함수
	static CSceneManager *GetInstance() {
		static CSceneManager _Instance;
		return &_Instance;
	};
#pragma endregion

private:
	IScene		*_pScene;	// 지금 진행중인 씬
	IScene		*_pResScene;// 예약중인 씬
	SceneState	_State;		// 지금 씬이 어떤상태인지
	SceneState	_ResState;	// 다음 씬이 어떤상태인지
	bool		_inGame;	// 게임이 진행중인 상태인가?
	bool		_reset;		// 게임이 리셋해야 하는지 리셋필요하면 true
public:
	// IScene 가상함수 실행
	void Init();//이 씬의 초기상태로
	void Update();//이 씬에서 매 프레임마다 해야할 일
	void Render();// 이 씬에서 매 프래임마다 화면에 출력할것
	void Destroy();// 이 씬이 끝날때 해야 할 일
public:
	void Reset();
	void Load(const WCHAR* scenename); // 등록된 신중 scenename 과 일치하는것을 _pResScene에 보유해둠
	void Change();	// 프레임의 첫 시작시 예약된 씬이 있으면 현재씬을 예약된 씬으로,  리셋을 해야하면 리셋을
	SceneState NowState() { return _State; }	// 현재씬을 반환하여 게임종료를 판단
	bool InGame();
};

