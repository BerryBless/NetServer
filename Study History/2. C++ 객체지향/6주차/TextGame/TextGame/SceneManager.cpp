#include "pch.h"
#include "SceneManager.h"
#include "DataManager.h"


SceneType	_sceneNow		= SceneType::Title;		// "현재" 게임씬 타입
int			_sceneStageNum	= 0;					// "현재" 플레이중인 스테이지

// ============================================================================
//							씬 로딩
// ---------------------------------------------------------------------------
// 씬의 흐름
// 1. Title		: Titel -> Load -> InGmae
// 2. Load		: Load -> Ingame
// 3. InGame	: InGame -> Load -> InGame -> Load -> ... -> InGame -> Ending
// 4. Ending	: Ending 출력하고 게임 종료
// type 씬으로 이동합니다.
// 다음 업데이트부터 적용됩니다.
// ============================================================================
void Scene_Load(SceneType type) {
	_sceneNow = type;
}

// ============================================================================
//							다음 스테이지 로딩
// ---------------------------------------------------------------------------
// _sceneStageNum를 하나 증가시키며 다음 스테이지를 로딩합니다.
// _sceneStageNum >= _stageDataCount 이면 엔딩을 로딩합니다.
// ============================================================================
void Scene_LoadNextStage() {
	_sceneStageNum++;
	if (_sceneStageNum >= _stageDataCount) {
		Scene_Load(SceneType::Ending);
	}
	else {
		Scene_Load(SceneType::Load);
	}
}