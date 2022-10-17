#include "pch.h"
#include "CSceneManager.h"

#include "CTitleScene.h"
#include "CGame.h"
#include "CEndingScene.h"

#include "Managers.h"
#include "CParser.h"
CSceneManager::CSceneManager() {
	_pScene = NULL;
	_pResScene = NULL;
	_inGame = true;
	_reset = false;
}
CSceneManager::~CSceneManager() {}


// 다형성
void CSceneManager::Init() {
	_pScene->Init();
}

void CSceneManager::Update() {
	_pScene->Update();
}

void CSceneManager::Render() {
	_pScene->Render();
}

void CSceneManager::Destroy() {
	if (_pScene == NULL) return;
	// 파괴
	// 1. 지금 관리하는 오브젝트를 모두 비운다.
	// 2. IScene의 Destroy를 실행한다
	// 3. 가지고있는 씬의 메모리할당을 해지한다.
	// 4. 파괴할 씬이 엔딩이라면 게임을 종료시킨다.
	I_FRAMEWORK->claer();
	_pScene->Destroy();
	delete _pScene;
	if (_State == SceneState::End) {
		_inGame = false;
	}
}

// 다음프레임에 리셋!
void CSceneManager::Reset() {
	_reset = true;
}

// 예약
// 씬을 추가할때마다 여기에 등록을 해야 그 씬으로 갈 수 있음
// scenename에 따라서
// _pResScene 에 new를 해주고,
// _ResState 를 바꿈
void CSceneManager::Load(const WCHAR *scenename) {

	if (Strcmp(scenename, L"title") == 0) {
		_pResScene = (IScene *) new CTitleScene;
		_ResState = SceneState::Title;
	} else if (Strcmp(scenename, L"stage1") == 0) {
		_pResScene = (IScene *) new CGame(L"Data/stage1.dat");
		_ResState = SceneState::Game;
	} else if (Strcmp(scenename, L"stage2") == 0) {
		_pResScene = (IScene *) new CGame(L"Data/stage2.dat");
		_ResState = SceneState::Game;
	} else if (Strcmp(scenename, L"ending") == 0) {
		_pResScene = (IScene *) new CEndingScene;
		_ResState = SceneState::End;
	}
}

// 진짜 바꾸기
// 메인루프의 첫번째줄 에서 실행
void CSceneManager::Change() {
	if (_pResScene != NULL) {
		// 예약된 씬이 있음
		// 1. 현재 씬을 파괴해주고 (Destroy )
		// 2. 예약한것을 사용할 수 있게 해주고
		// 3. 해당 씬을 초기상태로
		// 4. 예약을 해지

		Destroy();
		
		_pScene = _pResScene;
		_State = _ResState;
		
		_pScene->Init();
		
		_pResScene = NULL;
	} else if (_reset) {
		// 리셋을 해야하는 상황이면 _pScene->Init 호출
		_reset = false; 
		_pScene->Init();
	}
}

// 게임 진행중인지 확인
bool CSceneManager::InGame() {
	return _inGame;
}


