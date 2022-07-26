#include "pch.h"
#include "ContentCore.h"
#include "CFrameSkip.h"
#include "PacketProcess.h"
#include "Sector.h"
#include "NetworkCore.h"
// =========================
// 전역변수
// =========================

CObjectPool<CCharacter> g_characterPool(0, true);
std::map<DWORD, CCharacter *> g_characterMap; //캐릭터션맵
extern CFrameSkip g_Timer;
extern std::queue<SOCKET> g_delResvQ; // 삭제할 큐
int g_FixedUpdate = 0; // 1초에 updateCount 총횟수
BOOL g_dummytest = TRUE; // 더미테스트 on/off


BOOL ContentUpdate() {
	CCharacter *pCharacter = NULL;
	DWORD now = timeGetTime();
	// DELTATIME 오차
	int updateCount = g_Timer.FixedUpdateCount();

	if (updateCount != 1)
		_LOG(dfLOG_LEVEL_WARNING, L"updateCount[%d]", updateCount);
	else
		_LOG(dfLOG_LEVEL_DEBUG, L"updateCount[%d]", updateCount);

	if (updateCount < 1) return FALSE;

	for (int fixedupdate = 0; fixedupdate < updateCount; fixedupdate++) {
		g_FixedUpdate++;

		for (auto iter = g_characterMap.begin(); iter != g_characterMap.end(); ++iter) {
			// 캐릭터 하나하나 적절한 처리
			pCharacter = iter->second;
			if (pCharacter == NULL) { CRASH(); }

			if (!g_dummytest) {
				// 사망처리
				if (pCharacter->_chHP <= 0) {
					DisconnectClient(pCharacter);
					continue;
				}
			}

			//  일정시간 수신없음 종료
			if (now - pCharacter->_pSession->_LastRecvTime > dfMAX_IDLE_TIME) {
				DisconnectClient(pCharacter);
				continue;
			}

			_LOG(dfLOG_LEVEL_DEBUG, L"BEFORE UPDATE : SID[%d] ACTION[%d] POS[%d %d]", pCharacter->_SID, pCharacter->_action, pCharacter->_X, pCharacter->_Y);
			// 현재 동작에 따른 처리
			switch (pCharacter->_action) {
			case dfACTION_MOVE_LL:
				// 왼쪽
				if (CanGo(pCharacter->_X - (dfMOVE_PIXEL_HORIZONTAL),
					pCharacter->_Y)) {
					pCharacter->_X -= (dfMOVE_PIXEL_HORIZONTAL);
				}
				break;

			case dfACTION_MOVE_LU:
				// 왼쪽 위
				if (CanGo(pCharacter->_X - (dfMOVE_PIXEL_HORIZONTAL),
					pCharacter->_Y - (dfMOVE_PIXEL_VERTICAL))) {
					pCharacter->_X -= (dfMOVE_PIXEL_HORIZONTAL);
					pCharacter->_Y -= (dfMOVE_PIXEL_VERTICAL);
				}
				break;

			case dfACTION_MOVE_UU:
				// 위
				if (CanGo(pCharacter->_X,
					pCharacter->_Y - (dfMOVE_PIXEL_VERTICAL))) {
					pCharacter->_Y -= (dfMOVE_PIXEL_VERTICAL);
				}
				break;

			case dfACTION_MOVE_RU:
				// 오른쪽 위
				if (CanGo(pCharacter->_X + (dfMOVE_PIXEL_HORIZONTAL),
					pCharacter->_Y - (dfMOVE_PIXEL_VERTICAL))) {
					pCharacter->_X += (dfMOVE_PIXEL_HORIZONTAL);
					pCharacter->_Y -= (dfMOVE_PIXEL_VERTICAL);
				}
				break;

			case dfACTION_MOVE_RR:
				// 오른쪽
				if (CanGo(pCharacter->_X + (dfMOVE_PIXEL_HORIZONTAL),
					pCharacter->_Y)) {
					pCharacter->_X += (dfMOVE_PIXEL_HORIZONTAL);
				}
				break;

			case dfACTION_MOVE_RD:
				// 오른쪽 아래
				if (CanGo(pCharacter->_X + (dfMOVE_PIXEL_HORIZONTAL),
					pCharacter->_Y + (dfMOVE_PIXEL_VERTICAL))) {
					pCharacter->_X += (dfMOVE_PIXEL_HORIZONTAL);
					pCharacter->_Y += (dfMOVE_PIXEL_VERTICAL);
				}
				break;

			case dfACTION_MOVE_DD:
				// 아래
				if (CanGo(pCharacter->_X,
					pCharacter->_Y + (dfMOVE_PIXEL_VERTICAL))) {
					pCharacter->_Y += (dfMOVE_PIXEL_VERTICAL);
				}
				break;

			case dfACTION_MOVE_LD:
				// 왼쪽 아래
				if (CanGo(pCharacter->_X - (dfMOVE_PIXEL_HORIZONTAL),
					pCharacter->_Y + (dfMOVE_PIXEL_VERTICAL))) {
					pCharacter->_X -= (dfMOVE_PIXEL_HORIZONTAL);
					pCharacter->_Y += (dfMOVE_PIXEL_VERTICAL);
				}
				break;
			}
			_LOG(dfLOG_LEVEL_DEBUG, L"AFTER UPDATE : SID[%d] ACTION[%d] POS[%d %d]", pCharacter->_SID, pCharacter->_action, pCharacter->_X, pCharacter->_Y);
			//---------------------------------------------------------------
			// 이동인경우 섹터 업데이트
			//---------------------------------------------------------------
			if (dfACTION_MOVE_LL <= pCharacter->_action && pCharacter->_action <= dfACTION_MOVE_LD) {
				if (Sector_UpdateCharacter(pCharacter)) {
					ChatacterSectorUpdatePacket(pCharacter);
				}
			}
		}
	}
	return TRUE;
}



inline BOOL CanGo(int x, int y) {
	if (dfRANGE_MOVE_LEFT > x || x > dfRANGE_MOVE_RIGHT) return FALSE;
	if (dfRANGE_MOVE_TOP > y || y > dfRANGE_MOVE_BOTTOM) return FALSE;
	return TRUE;
}


void HitScanAndSend(CCharacter *pAttacker) {
	st_PACKET_HEADER header;
	CPacket packet;
	int dmg;
	int rangeX;
	int rangeY;
	int sectorX;
	int sectorY;
	std::list<CCharacter *> *pSectorList;
	CCharacter *pCharacter;
	// --------------------------------------------
	// 	   섹터 업데이트
	// --------------------------------------------
	if (Sector_UpdateCharacter(pAttacker)) {
		ChatacterSectorUpdatePacket(pAttacker);
	}
	// --------------------------------------------
	// 	   액션 패킷 보내기
	// --------------------------------------------
	switch (pAttacker->_action) {
	case dfACTION_ATTACK1:
		MakePacket_Action_Attack1(&header, &packet, pAttacker->_SID, pAttacker->_direction, pAttacker->_X, pAttacker->_Y);
		dmg = dfATTACK1_DAMAGE;
		rangeX = dfATTACK1_RANGE_X;
		rangeY = dfATTACK1_RANGE_Y;
		break;
	case dfACTION_ATTACK2:
		MakePacket_Action_Attack2(&header, &packet, pAttacker->_SID, pAttacker->_direction, pAttacker->_X, pAttacker->_Y);
		dmg = dfATTACK2_DAMAGE;
		rangeX = dfATTACK2_RANGE_X;
		rangeY = dfATTACK2_RANGE_Y;
		break;
	case dfACTION_ATTACK3:
		MakePacket_Action_Attack3(&header, &packet, pAttacker->_SID, pAttacker->_direction, pAttacker->_X, pAttacker->_Y);
		dmg = dfATTACK3_DAMAGE;
		rangeX = dfATTACK3_RANGE_X;
		rangeY = dfATTACK3_RANGE_Y;
		break;
	default:
		_LOG(dfLOG_LEVEL_ERROR, L"HitScanAndSend( SID[%d] Action[%d] ):: NOT ACTTECK", pAttacker->_SID, pAttacker->_action);
		return;
	}
	SendSectorAround(pAttacker, &header, &packet);
	// --------------------------------------------
	// 	   데미지 판정
	// --------------------------------------------
	if (pAttacker->_direction == dfACTION_MOVE_LL) {
		// 왼쪽판정

		sectorX = pAttacker->_curSecPos._X;
		sectorY = pAttacker->_curSecPos._Y;


		// 내가있는 섹터
		pSectorList = &g_sector[sectorY][sectorX];
		for (auto iter = pSectorList->begin(); iter != pSectorList->end(); ++iter) {
			pCharacter = *iter;
			if ((pAttacker->_X - pCharacter->_X) <= rangeX &&
				(pAttacker->_X - pCharacter->_X) > 0 &&
				abs(pAttacker->_Y - pCharacter->_Y) <= rangeY) {
				pCharacter->_chHP -= dmg;
				if (pCharacter->_chHP < 0)
					pCharacter->_chHP = 0;
				MakePacket_Damage(&header, &packet, pAttacker->_SID, pCharacter->_SID, pCharacter->_chHP);
				SendSectorAround(pCharacter, &header, &packet, TRUE);
			}
		}

		// 인접한 섹터 (왼쪽만)
		sectorX -= 1;
		sectorY -= 1;

		if (sectorX >= 0) {
			for (int i = 0; i < 3; ++i, ++sectorY) {
				if (sectorY < 0 || sectorY >= dfSECTOR_MAX_Y) {
					continue;
				}

				pSectorList = &g_sector[sectorY][sectorX];
				for (auto iter = pSectorList->begin(); iter != pSectorList->end(); ++iter) {
					pCharacter = *iter;

					if ((pAttacker->_X - pCharacter->_X) <= rangeX &&
						(pAttacker->_X - pCharacter->_X) > 0 &&
						abs(pAttacker->_Y - pCharacter->_Y) <= rangeY) {
						pCharacter->_chHP -= dmg;
						if (pCharacter->_chHP < 0)
							pCharacter->_chHP = 0;
						MakePacket_Damage(&header, &packet, pAttacker->_SID, pCharacter->_SID, pCharacter->_chHP);
						SendSectorAround(pCharacter, &header, &packet, TRUE);
					}
				}
			}
		}


	} else if (pAttacker->_direction == dfACTION_MOVE_RR) {
		// 오른쪽판정

		sectorX = pAttacker->_curSecPos._X;
		sectorY = pAttacker->_curSecPos._Y;


		// 내가있는 섹터
		pSectorList = &g_sector[sectorY][sectorX];
		for (auto iter = pSectorList->begin(); iter != pSectorList->end(); ++iter) {
			pCharacter = *iter;
			if ((pCharacter->_X - pAttacker->_X) <= rangeX &&
				(pCharacter->_X - pAttacker->_X) > 0 &&
				abs(pAttacker->_Y - pCharacter->_Y) <= rangeY) {
				pCharacter->_chHP -= dmg;
				if (pCharacter->_chHP < 0)
					pCharacter->_chHP = 0;
				MakePacket_Damage(&header, &packet, pAttacker->_SID, pCharacter->_SID, pCharacter->_chHP);
				SendSectorAround(pCharacter, &header, &packet, TRUE);
			}
		}

		// 인접한 섹터 (오른쪽만)
		sectorX += 1;
		sectorY -= 1;

		if (sectorX < dfSECTOR_MAX_X) {
			for (int i = 0; i < 3; ++i, ++sectorY) {
				if (sectorY < 0 || sectorY >= dfSECTOR_MAX_Y) {
					continue;
				}

				pSectorList = &g_sector[sectorY][sectorX];
				for (auto iter = pSectorList->begin(); iter != pSectorList->end(); ++iter) {
					pCharacter = *iter;

					if ((pCharacter->_X - pAttacker->_X) <= rangeX &&
						(pCharacter->_X - pAttacker->_X) > 0 &&
						abs(pAttacker->_Y - pCharacter->_Y) <= rangeY) {
						pCharacter->_chHP -= dmg;
						if (pCharacter->_chHP < 0)
							pCharacter->_chHP = 0;
						MakePacket_Damage(&header, &packet, pAttacker->_SID, pCharacter->_SID, pCharacter->_chHP);
						SendSectorAround(pCharacter, &header, &packet, TRUE);
					}
				}
			}
		}
	}
}

void DisconnectClient(CCharacter *pCharacter) {
	//---------------------------
	// 섹터에 삭제 메시지 뿌리기
	//---------------------------
	// 패킷 
	st_PACKET_HEADER header;
	CPacket packet;

	MakePacket_Delete_Character(&header, &packet, pCharacter->_SID);

	SendSectorAround(pCharacter, &header, &packet);

	// 세션 삭제 큐에 넣기
	g_delResvQ.push(pCharacter->_pSession->_sock);

}


#pragma region WRAPPING
// 자료구조 컨테이너 래핑
CCharacter *CreateCharacter(DWORD SID) {
	//---------------------------
	// 	   캐릭 생성
	//---------------------------

// 이미 있는 세션인지 확인
	auto iter = g_characterMap.find(SID);
	if (iter != g_characterMap.end()) {
		_LOG(dfLOG_LEVEL_ERROR, L"////////CreateCharacter() Session that already exists..");
		return NULL;
	}

	// 오브젝트풀에서 하나 뽑아오기
	CCharacter *pCharacter = g_characterPool.Alloc();

	// 컨테이너에 넣기
	g_characterMap.insert(std::make_pair(SID, pCharacter));
	_LOG(dfLOG_LEVEL_DEBUG, L"CreateCharacter :: SID[%d] OK..", SID);
	return pCharacter;

}
CCharacter *FindCharacter(DWORD SID) {
	//---------------------------
	// 	   캐릭 검색
	//---------------------------

	// 이미 있는 캐릭인지 확인
	auto iter = g_characterMap.find(SID);
	if (iter == g_characterMap.end()) {
		// 없음 NULL
		_LOG(dfLOG_LEVEL_ERROR, L"////////FindCharacter() Session not found..");
		return NULL;
	}
	// 있음 세션
	_LOG(dfLOG_LEVEL_DEBUG, L"FindCharacter() OK..");
	return iter->second;
}
BOOL DeleteCharacter(DWORD SID) {
	//---------------------------
	// 	   캐릭 삭제
	//---------------------------

// 이미 있는 세션인지 확인
	auto iter = g_characterMap.find(SID);
	if (iter == g_characterMap.end()) {
		// 존재하지 않는 세션 삭제못함
		_LOG(dfLOG_LEVEL_ERROR, L"////////DeleteCharacter() Session not found..");
		return FALSE;
	}

	// 섹터에서 삭제
	Sector_RemoveCharacter(iter->second);

	// 오브젝트 풀에 반환
	g_characterPool.Free(iter->second);
	// 컨테이너에서 삭제
	g_characterMap.erase(iter);

	_LOG(dfLOG_LEVEL_DEBUG, L"DeleteCharacter() OK..");
	return true;
}
#pragma endregion
