#include "CFramework.h"
#include "Managers.h"
#include "NetworkCore.h"
#include "PacketProcess.h"
//#define TEMP // 임시 객체
BYTE  m_byTileMap[dfMAP_HEIGHT][dfMAP_WIDTH];

CFramework::CFramework() {

	Init();
}
CFramework::~CFramework() {
	DESTROY(_opCamera);
}


void CFramework::Init() {
	_objList.clear();
	_opCamera = (CCameraObject *) this->Instantiate(new CCameraObject);
	_llFrameTotal = 0;

	memset(m_byTileMap, 0, dfMAP_WIDTH * dfMAP_HEIGHT);

#ifdef TEMP
	_iObjectIDCounter = 0;


	INSTANTIATE(new CPlayerObject(_iObjectIDCounter++, 200, 150, FALSE));
	INSTANTIATE(new CEffectObject(1, 200, 120));
	INSTANTIATE(new CPlayerObject(_iObjectIDCounter++, 200, 190, FALSE));
	INSTANTIATE(new CPlayerObject(_iObjectIDCounter++, 200, 130, FALSE));
	_opPlayer = (CPlayerObject *) INSTANTIATE(new CPlayerObject(_iObjectIDCounter++, 200, 110, TRUE));
#endif // TEMP


#pragma region Load Sprite
	// 모든 스프라이트 불러오기
	CSpriteDib *spriteInst = I_SPRITEDIB;

	// MAP
	spriteInst->LoadDibSprite(e_SPRITE::eTileMap, L"Sprite_Data/Tile_01.bmp", 0, 0);
	spriteInst->LoadDibSprite(e_SPRITE::eMap, L"Sprite_Data/_Map.bmp", 0, 0);
	// stand
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_STAND_L01, L"Sprite_Data/Stand_L_01.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_STAND_L02, L"Sprite_Data/Stand_L_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_STAND_L03, L"Sprite_Data/Stand_L_03.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_STAND_L04, L"Sprite_Data/Stand_L_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_STAND_L05, L"Sprite_Data/Stand_L_01.bmp", 71, 90);

	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_STAND_R01, L"Sprite_Data/Stand_R_01.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_STAND_R02, L"Sprite_Data/Stand_R_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_STAND_R03, L"Sprite_Data/Stand_R_03.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_STAND_R04, L"Sprite_Data/Stand_R_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_STAND_R05, L"Sprite_Data/Stand_R_01.bmp", 71, 90);
	// Move
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L01, L"Sprite_Data/Move_L_01.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L02, L"Sprite_Data/Move_L_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L03, L"Sprite_Data/Move_L_03.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L04, L"Sprite_Data/Move_L_04.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L05, L"Sprite_Data/Move_L_05.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L06, L"Sprite_Data/Move_L_06.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L07, L"Sprite_Data/Move_L_07.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L08, L"Sprite_Data/Move_L_08.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L09, L"Sprite_Data/Move_L_09.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L10, L"Sprite_Data/Move_L_10.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L11, L"Sprite_Data/Move_L_11.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_L12, L"Sprite_Data/Move_L_12.bmp", 71, 90);

	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R01, L"Sprite_Data/Move_R_01.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R02, L"Sprite_Data/Move_R_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R03, L"Sprite_Data/Move_R_03.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R04, L"Sprite_Data/Move_R_04.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R05, L"Sprite_Data/Move_R_05.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R06, L"Sprite_Data/Move_R_06.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R07, L"Sprite_Data/Move_R_07.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R08, L"Sprite_Data/Move_R_08.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R09, L"Sprite_Data/Move_R_09.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R10, L"Sprite_Data/Move_R_10.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R11, L"Sprite_Data/Move_R_11.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLATER_MOVE_R12, L"Sprite_Data/Move_R_12.bmp", 71, 90);
	// Attack 1
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK1_L01, L"Sprite_Data/Attack1_L_01.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK1_L02, L"Sprite_Data/Attack1_L_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK1_L03, L"Sprite_Data/Attack1_L_03.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK1_L04, L"Sprite_Data/Attack1_L_04.bmp", 71, 90);

	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK1_R01, L"Sprite_Data/Attack1_R_01.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK1_R02, L"Sprite_Data/Attack1_R_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK1_R03, L"Sprite_Data/Attack1_R_03.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK1_R04, L"Sprite_Data/Attack1_R_04.bmp", 71, 90);

	// Attack 2
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK2_L01, L"Sprite_Data/Attack2_L_01.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK2_L02, L"Sprite_Data/Attack2_L_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK2_L03, L"Sprite_Data/Attack2_L_03.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK2_L04, L"Sprite_Data/Attack2_L_04.bmp", 71, 90);

	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK2_R01, L"Sprite_Data/Attack2_R_01.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK2_R02, L"Sprite_Data/Attack2_R_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK2_R03, L"Sprite_Data/Attack2_R_03.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK2_R04, L"Sprite_Data/Attack2_R_04.bmp", 71, 90);

	// Attack 3
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_L01, L"Sprite_Data/Attack3_L_01.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_L02, L"Sprite_Data/Attack3_L_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_L03, L"Sprite_Data/Attack3_L_03.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_L04, L"Sprite_Data/Attack3_L_04.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_L05, L"Sprite_Data/Attack3_L_05.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_L06, L"Sprite_Data/Attack3_L_06.bmp", 71, 90);

	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_R01, L"Sprite_Data/Attack3_R_01.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_R02, L"Sprite_Data/Attack3_R_02.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_R03, L"Sprite_Data/Attack3_R_03.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_R04, L"Sprite_Data/Attack3_R_04.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_R05, L"Sprite_Data/Attack3_R_05.bmp", 71, 90);
	spriteInst->LoadDibSprite(e_SPRITE::ePLAYER_ATTACK3_R06, L"Sprite_Data/Attack3_R_06.bmp", 71, 90);

	// Effect
	spriteInst->LoadDibSprite(e_SPRITE::eEFFECT_SPARK_01, L"Sprite_Data/xSpark_1.bmp", 70, 70);
	spriteInst->LoadDibSprite(e_SPRITE::eEFFECT_SPARK_02, L"Sprite_Data/xSpark_2.bmp", 70, 70);
	spriteInst->LoadDibSprite(e_SPRITE::eEFFECT_SPARK_03, L"Sprite_Data/xSpark_3.bmp", 70, 70);
	spriteInst->LoadDibSprite(e_SPRITE::eEFFECT_SPARK_04, L"Sprite_Data/xSpark_4.bmp", 70, 70);

	// UI
	spriteInst->LoadDibSprite(e_SPRITE::eGUAGE_HP, L"Sprite_Data/HPGuage.bmp", 0, 0);
	spriteInst->LoadDibSprite(e_SPRITE::eSHADOW, L"Sprite_Data/Shadow.bmp", 30, 4);
#pragma endregion


}

#pragma region Comp
bool BaseObjectComp_Pos(CBaseObject *obj1, CBaseObject *obj2) {
	// Y값이 큰게 맨마지막
	return obj1->GetCurY() < obj2->GetCurY();
}
bool BaseObjectComp_Type(CBaseObject *obj1, CBaseObject *obj2) {
	// 이펙트면 맨 마지막으로
	return obj1->GetObjectType() != (int) CBaseObject::e_OBJECTTYPE::EFFECT;
}
#pragma endregion

void CFramework::Update() {
	_llFrameTotal++;// 프레임 새기
	// 각 겍체의 업데이트
	for (auto iter = _objList.begin(); iter != _objList.end(); ) {
		// 객체가 살아있으면 업데이트
		if ((*iter)->IsActivate()) {
			(*iter)->Update();
			++iter;
		} else {
			// 죽었으면 삭제
			delete *iter;
			iter = _objList.erase(iter);
		}
	}

	// Y축기준으로 정렬
	_objList.sort(BaseObjectComp_Pos);

	// 이펙트 객체는 맨뒤로(가장 나중에 랜더링)
	_objList.sort(BaseObjectComp_Type);
}

BOOL CFramework::FrameSkip() {
	return _frameSkip.FrameSkip();
}

int CFramework::GetRenderFPS() {
	return _frameSkip.GetRenderFPS();
}

int CFramework::GetLogicFPS() {
	return _frameSkip.GetLogicFPS();
}

void CFramework::SetPlayer(CPlayerObject *opPlayer) {
	if (opPlayer == NULL) return;
	_opPlayer = opPlayer;
	_opCamera->SetTarget(opPlayer);
}

// 생성
CBaseObject *CFramework::Instantiate(CBaseObject *bObj) {
	// 리스트에 등록
	_objList.push_back(bObj);
	return bObj;
}

// 삭제!
void CFramework::Destroy(CBaseObject *bObj) {
	// 찾아서 (*iter)->Destroy() 호출
	// TODO delete큐 만들기
	for (auto iter = _objList.begin(); iter != _objList.end(); ++iter) {
		if ((*iter) == bObj) {
			(*iter)->Destroy();
		}
	}
}
CBaseObject *CFramework::FindObject(int iID) {
	for (auto iter = _objList.begin(); iter != _objList.end(); ++iter) {
		if ((*iter)->GetObjectID() == iID) {
			return (*iter);
		}
	}

	return nullptr;
}


void CFramework::Render() {
	// 싱글톤 얻어오기
	CScreenDib *scrrenInst = I_SCREENDIB;
	CSpriteDib *spriteInst = I_SPRITEDIB;

	// 출력위치(화면크기) 얻어오기
	BYTE *pDest = scrrenInst->GetDibBuffer(); //DibBuffer
	int iWidth = scrrenInst->GetWidth();		
	int iHeight = scrrenInst->GetHeight();
	int iPitch = scrrenInst->GetPitch();



	// TODO 내플레이어 기준 카메라좌표 계산
	// 카메라 좌표 기준의 타일맵 출력

	int worldObjectXPos;
	int worldObjectYPos;

	if (this->_opPlayer == NULL)  return;
 
	/// 타일맵 그리기
		int cameraXPos = this->_opCamera->GetCurX();
		int cameraYPos = this->_opCamera->GetCurY();

		int tileXIndex = 0;
		int tileYIndex = 0;
		int tileXOverFlow = 0;
		int tileYOverFlow = 0;

		// 타일 인덱스 계산
		tileXIndex = cameraXPos / 64;
		tileYIndex = cameraYPos / 64;
		tileXOverFlow = cameraXPos % 64;
		tileYOverFlow = cameraYPos % 64;
		
		// 타일 출력
		for (int i = 0; i < 11; i++) {
			tileXIndex = cameraXPos / 64;
			for (int j = 0; j < 11; j++) {
				spriteInst->DrawSprite
				(m_byTileMap[tileYIndex][tileXIndex++], 
					j * 64 - tileXOverFlow, i * 64 - tileYOverFlow,
					pDest, iWidth, iHeight, iPitch,100,TRUE);
			}
			tileYIndex++;
		}
	// 맵그리기
	//spriteInst->DrawSprite(e_SPRITE::eMap, 0, 0, pDest, iWidth, iHeight, iPitch);

	// 오브젝트 그리기
		int screenObjectXPos;
		int screenObjectYPos;
	for (auto iter = _objList.begin(); iter != _objList.end(); ++iter) {
		// TODO 내월드좌표->카메라 기준의 모니터좌표로
		// 카메라좌표기준 랜더링

		worldObjectXPos = (*iter)->GetCurX();
		worldObjectYPos = (*iter)->GetCurY();

		screenObjectXPos = worldObjectXPos - cameraXPos;
		screenObjectYPos = worldObjectYPos - cameraYPos;

		(*iter)->SetPosition(screenObjectXPos, screenObjectYPos);
		(*iter)->Render(pDest, iWidth, iHeight, iPitch);
		(*iter)->SetPosition(worldObjectXPos, worldObjectYPos);
	}

}


BOOL g_bActiveApp;// 이 윈도우가 활성화 되있냐 
void CFramework::KeyProcess() {
	if (g_bActiveApp == FALSE) return;
	if (_opPlayer == NULL) return;
	if (_opPlayer->GetCurAction() == CPlayerObject::e_PLAYERACTION::ATTACK) return;
	DWORD dwInputMessage = dfACTION_STAND;


	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		// 왼쪽
		dwInputMessage = dfACTION_MOVE_LL;

		if (GetAsyncKeyState(VK_UP) & 0x8000) {
			// 왼쪽위
			dwInputMessage = dfACTION_MOVE_LU;
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
			// 왼쪽아래
			dwInputMessage = dfACTION_MOVE_LD;
		}

	} else if (GetAsyncKeyState(VK_UP) & 0x8000) {
		// 위
		dwInputMessage = dfACTION_MOVE_UU;

		if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			// 왼쪽위
			dwInputMessage = dfACTION_MOVE_LU;
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			// 오른쪽위
			dwInputMessage = dfACTION_MOVE_RU;
		}

	} else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		// 오른쪽
		dwInputMessage = dfACTION_MOVE_RR;

		if (GetAsyncKeyState(VK_UP) & 0x8000) {
			// 오른쪽위
			dwInputMessage = dfACTION_MOVE_RU;
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
			// 오른쪽아래
			dwInputMessage = dfACTION_MOVE_RD;
		}
	} else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
		// 아래
		dwInputMessage = dfACTION_MOVE_DD;

		if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			// 왼쪽아래
			dwInputMessage = dfACTION_MOVE_LD;
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			// 오른쪽아래
			dwInputMessage = dfACTION_MOVE_RD;
		}
	}

	if (GetAsyncKeyState('Z') & 0x8000) {
		dwInputMessage = dfACTION_ATTACK1;
	} else if (GetAsyncKeyState('X') & 0x8000) {
		dwInputMessage = dfACTION_ATTACK2;
	} else if (GetAsyncKeyState('C') & 0x8000) {
		dwInputMessage = dfACTION_ATTACK3;
	}



	_opPlayer->ActionInput(dwInputMessage);
}
