#include "pch.h"
#include "CTitleScene.h"
#include "Managers.h"
void CTitleScene::Update() {
	// 엔터키를 기다립니다.
	if (GetAsyncKeyState(VK_RETURN) & 0x8001) {
		// 엔터키가 눌렸으니 스테이지 로딩하라고 알려줍니다
		I_SCENEMANAGER->Load(L"stage1");
	}
}

void CTitleScene::Render() {
	CScreenBuffer *scrInst = I_SCREENBUFFER;
	scrInst->Sprite_Draw(0, 0, "Title Scene", 11);
	scrInst->Sprite_Draw(0, 1, "Move   : Arrow keys", 17);
	scrInst->Sprite_Draw(0, 2, "Attack : W, A, S, D", 19);
	scrInst->Sprite_Draw(0, 3, "P -> @", 6);
	scrInst->Sprite_Draw(0, 4, ">> Press Enter <<", 17);
}
