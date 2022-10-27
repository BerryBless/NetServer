#include "pch.h"
#include "CEndingScene.h"
#include "Managers.h"
void CEndingScene::Update() {
	// 엔터키를 기다립니다.
	if (GetAsyncKeyState(VK_RETURN) & 0x8001) {
		// 엔터가 눌리면 게임종료
		I_SCENEMANAGER->Destroy();
	}
}

void CEndingScene::Render() {
	CScreenBuffer *scrInst = I_SCREENBUFFER;

	scrInst->Sprite_Draw(0, 0, "Ending Scene", 12);
	scrInst->Sprite_Draw(0, 1, "YOU WON!!", 9);
	scrInst->Sprite_Draw(0, 4, ">> Press Enter <<", 17);
}
