#include "pch.h"
#include "Sector.h"
#include "Character.h"
#include "ContentCore.h"
#include "NetworkCore.h"
#include "PacketProcess.h"
// 캐릭터맵
extern std::map<DWORD, CCharacter *> g_characterMap;
// 월드맵 캐릭터 섹터
std::list<CCharacter *> g_sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

void Sector_AddCharacter(CCharacter *pCharacter) {
	if (pCharacter == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// Sector_AddCharacter : pCharacter IS NULL." );
		CRASH();
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"Sector_AddCharacter :: SID[%d]\n", pCharacter->_SID);
	_LOG(dfLOG_LEVEL_DEBUG, L"World Pos (%d, %d)", pCharacter->_X, pCharacter->_Y);

	// 섹터좌표 구하기
	int sectorX = pCharacter->_X / dfSECTOR_WIDTH;
	int sectorY = pCharacter->_Y / dfSECTOR_HEIGHT;

	// 섹터 좌표 저장
	pCharacter->_curSecPos._X = sectorX;
	pCharacter->_curSecPos._Y = sectorY;

	// 해당좌표 섹터에 등록
	g_sector[sectorY][sectorX].push_back(pCharacter);

	_LOG(dfLOG_LEVEL_DEBUG, L"Sector Pos (%d, %d)\n", pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y);
}

void Sector_RemoveCharacter(CCharacter *pCharacter) {
	if (pCharacter == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// Sector_RemoveCharacter : pCharacter IS NULL.");
		CRASH();
		return;
	}




	// 삭제할 섹터의 캐릭터 리스트
	std::list<CCharacter *> *pSectorList = &g_sector[pCharacter->_curSecPos._Y][pCharacter->_curSecPos._X];

	// pCharacter 한번 찾아보기
	auto iter = std::find(pSectorList->begin(), pSectorList->end(), pCharacter);
	
	// 에러 찾을 수 없는 캐릭터
	if (iter == pSectorList->end()) {
		_LOG(dfLOG_LEVEL_ERROR, L"///////// Sector_RemoveCharacter :: I can't find the character.\n\tSID[%d] \n\tWorld Pos (%d, %d) , Sector Pos (%d, %d)",
			pCharacter->_SID, pCharacter->_X, pCharacter->_Y, pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y);
		CRASH();
	}
	
	// 삭제
	pSectorList->erase(iter);

	_LOG(dfLOG_LEVEL_DEBUG, L"Sector_RemoveCharacter :: SID[%d]\nWorld Pos (%d, %d) , Sector Pos (%d, %d)",
		pCharacter->_SID, pCharacter->_X, pCharacter->_Y, pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y);
}

BOOL Sector_UpdateCharacter(CCharacter *pCharacter) {
	if (pCharacter == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// Sector_UpdateCharacter : pCharacter IS NULL.");
		CRASH();
		return FALSE;
	}


	_LOG(dfLOG_LEVEL_DEBUG, L"Sector_UpdateCharacter :: SID[%d]\nWorld Pos (%d, %d) , Before Sector Pos (%d, %d)",
		pCharacter->_SID, pCharacter->_X, pCharacter->_Y, pCharacter->_oldSecPos._X, pCharacter->_oldSecPos._Y);


	// 	   	// 섹터좌표 구하기
	int sectorX = pCharacter->_X / dfSECTOR_WIDTH;
	int sectorY = pCharacter->_Y / dfSECTOR_HEIGHT;
	// 1. 좌표가 안움직였을
	if (sectorX == pCharacter->_curSecPos._X && sectorY == pCharacter->_curSecPos._Y)
		return FALSE;
	// 2. 좌표가 움직였을때
	Sector_RemoveCharacter(pCharacter);

	pCharacter->_oldSecPos._X = pCharacter->_curSecPos._X;
	pCharacter->_oldSecPos._Y = pCharacter->_curSecPos._Y;

	Sector_AddCharacter(pCharacter);

	_LOG(dfLOG_LEVEL_DEBUG, L"After Sector Pos (%d, %d)\n", pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y);
	return TRUE;
}

void GetSectorAround(int iSecX, int iSecY, st_SECTOR_AROUND *pSectorAround) {
	if (iSecX < 0 || iSecX > dfSECTOR_MAX_X ||
		iSecY < 0 || iSecY > dfSECTOR_MAX_Y) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// GetSectorAround : out of range. (%d , %d)", iSecX, iSecY);
		CRASH();
	}
	if (pSectorAround == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// GetSectorAround : pSectorAround IS NULL.");
		CRASH();
	}


	// (iSecX, iSecY) 을 둘러싸는 9칸 뽑기
	_LOG(dfLOG_LEVEL_DEBUG, L"GetSectorAround :: (iSecX, iSecY) [%d, %d]\n", iSecX, iSecY);
	iSecX--;
	iSecY--;
	pSectorAround->_count = 0;

	for (int y = 0; y < 3; y++) {
		if (iSecY + y < 0 ||
			iSecY + y >= dfSECTOR_MAX_Y)
			continue;
		for (int x = 0; x < 3; x++) {
			if (iSecX + x < 0 ||
				iSecX + x >= dfSECTOR_MAX_X)
				continue;

			pSectorAround->_around[pSectorAround->_count]._X = iSecX + x;
			pSectorAround->_around[pSectorAround->_count]._Y = iSecY + y;
			pSectorAround->_count++;
		}
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"pSectorAround->_count [%d]", pSectorAround->_count);
	for (int i = 0; i < pSectorAround->_count; i++) {
		_LOG(dfLOG_LEVEL_DEBUG, L"pSectorAround->_around[%d] :: (%d, %d)", i, pSectorAround->_around[i]._X, pSectorAround->_around[i]._Y);
	}
}

void GetUpdateSectorAround(CCharacter *pCharacter, st_SECTOR_AROUND *pRemoveSector, st_SECTOR_AROUND *pAddSector) {
	if (pCharacter == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// GetUpdateSectorAround : pCharacter IS NULL.");
		CRASH();
	}
	if (pRemoveSector == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// GetUpdateSectorAround : pRemoveSector IS NULL.");
		CRASH();
	}
	if (pAddSector == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// GetUpdateSectorAround : pAddSector IS NULL.");
		CRASH();
	}



	_LOG(dfLOG_LEVEL_DEBUG, L"GetUpdateSectorAround :: SID[%d], SecPos [%d, %d]\n",
		pCharacter->_SID, pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y);
	//TODO
	BOOL find;
	st_SECTOR_AROUND oldAround;
	st_SECTOR_AROUND curAround;

	pRemoveSector->_count = 0;
	pAddSector->_count = 0;

	GetSectorAround(pCharacter->_oldSecPos._X, pCharacter->_oldSecPos._Y, &oldAround);
	GetSectorAround(pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y, &curAround);

	// Old섹터 정보중 add에는 없는 정보를 찾아서 remove에 넣음
	for (int oldCnt = 0; oldCnt < oldAround._count; oldCnt++) {
		find = false;
		for (int curCnt = 0; curCnt < curAround._count; curCnt++) {

			if (oldAround._around[oldCnt]._X == curAround._around[curCnt]._X &&
				oldAround._around[oldCnt]._Y == curAround._around[curCnt]._Y) {
				find = true;
				break;
			}
		}
		if (find == false) {
			pRemoveSector->_around[pRemoveSector->_count] = oldAround._around[oldCnt];
			pRemoveSector->_count++;
		}
	}

	// add섹터 정보중 old에는 없는 정보를 찾아서 add에 넣음
	for (int curCnt = 0; curCnt < curAround._count; curCnt++) {
		find = false;
		for (int oldCnt = 0; oldCnt < oldAround._count; oldCnt++) {
			if (oldAround._around[oldCnt]._X == curAround._around[curCnt]._X &&
				oldAround._around[oldCnt]._Y == curAround._around[curCnt]._Y) {
				find = true;
				break;
			}
		}
		if (find == false) {
			pAddSector->_around[pAddSector->_count] = curAround._around[curCnt];
			pAddSector->_count++;
		}
	}


	_LOG(dfLOG_LEVEL_DEBUG, L"pRemoveSector->_count [%d]", pRemoveSector->_count);
	for (int i = 0; i < pRemoveSector->_count; i++) {
		_LOG(dfLOG_LEVEL_DEBUG, L"pRemoveSector->_around[%d] :: (%d, %d)", i, pRemoveSector->_around[i]._X, pRemoveSector->_around[i]._Y);
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"pAddSector->_count [%d]", pAddSector->_count);
	for (int i = 0; i < pAddSector->_count; i++) {
		_LOG(dfLOG_LEVEL_DEBUG, L"pAddSector->_around[%d] :: (%d, %d)", i, pAddSector->_around[i]._X, pAddSector->_around[i]._Y);
	}
}

void ChatacterSectorUpdatePacket(CCharacter *pCharacter) {
	if (pCharacter == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// ChatacterSectorUpdatePacket : pCharacter IS NULL.");
		CRASH();
	}


	_LOG(dfLOG_LEVEL_DEBUG, L"ChatacterSectorUpdatePacket :: SID[%d], oldSecPos [%d, %d], curSecPos [%d, %d]\n",
		pCharacter->_SID, pCharacter->_oldSecPos._X, pCharacter->_oldSecPos._Y, pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y);
	st_SECTOR_AROUND RemoveSector, AddSector; // 삭제될 / 추가될 섹터
	CCharacter *pExistCharacter;	// 섹터에 있던 캐릭터

	// 섹터에 있는 캐릭터 가르키기
	std::list<CCharacter *> *pSectorList;

	// 패킷 
	st_PACKET_HEADER header;
	CPacket packet;

	GetUpdateSectorAround(pCharacter, &RemoveSector, &AddSector);

	// -----------------------------------------------
	// 1. 이전 섹터에서 없어진 부분에 - 케릭터 삭제 메시지
	// -----------------------------------------------
	MakePacket_Delete_Character(&header, &packet, pCharacter->_SID);

	for (int i = 0; i < RemoveSector._count; i++) {
		SendSectorOne(
			RemoveSector._around[i]._X, RemoveSector._around[i]._Y,
			NULL,
			&header, &packet);
	}

	// -----------------------------------------------
	// 2. 이동하는 캐릭터에게 이전섹터에서 제외된 부분의 캐릭터의 삭제메시지
	// -----------------------------------------------
	for (int i = 0; i < RemoveSector._count; i++) {
		pSectorList = &g_sector[RemoveSector._around[i]._Y][RemoveSector._around[i]._X];
		for (auto iter = pSectorList->begin(); iter != pSectorList->end(); ++iter) {
			MakePacket_Delete_Character(&header, &packet, (*iter)->_SID);
			SendUnicast(pCharacter->_pSession, &header, &packet);
		}
	}


	// -----------------------------------------------
	// 3. 새로추가된 섹터에 - 캐릭터 생성 + 이동 메시지
	// -----------------------------------------------

	// -----------------------------------------------
	// 3.1 새로추가된 섹터에 - 캐릭터 생성 
	// -----------------------------------------------
	MakePacket_Create_Other_Character(&header, &packet,
		pCharacter->_SID,
		pCharacter->_direction,
		pCharacter->_X,
		pCharacter->_Y,
		pCharacter->_chHP);

	for (int i = 0; i < AddSector._count; i++) {
		SendSectorOne(
			AddSector._around[i]._X, AddSector._around[i]._Y,
			NULL,
			&header, &packet);
	}

	// -----------------------------------------------
	// 3.2 새로추가된 섹터에 - 생성된 캐릭터 이동 패킷
	// 	   항상이동중임
	// -----------------------------------------------

	MakePacket_Move_Start(&header, &packet,
		pCharacter->_SID,
		pCharacter->_moveDirection,
		pCharacter->_X,
		pCharacter->_Y);

	for (int i = 0; i < AddSector._count; i++) {
		SendSectorOne(
			AddSector._around[i]._X, AddSector._around[i]._Y,
			NULL,
			&header, &packet);
	}

	// -----------------------------------------------
	// 4. 이동하는 캐릭터에게 - 새로 진입한 섹터의 캐릭터 생성 메시지
	// -----------------------------------------------
	for (int i = 0; i < AddSector._count; i++) {
		// 얻어진 섹터 리스트 접근
		pSectorList = &g_sector[AddSector._around[i]._Y][AddSector._around[i]._X];
		// 해당 섹터에 등록된 캐릭터를 뽑아서 생성패킷 만들어보냄
		for (auto iter = pSectorList->begin(); iter != pSectorList->end(); ++iter) {
			pExistCharacter = *iter;

			if (pExistCharacter == pCharacter) {
				// 나는 안보내도됨
				continue;
			}

			MakePacket_Create_Other_Character(&header, &packet,
				pExistCharacter->_SID,
				pExistCharacter->_direction,
				pExistCharacter->_X,
				pExistCharacter->_Y,
				pExistCharacter->_chHP);

			SendUnicast(pCharacter->_pSession, &header, &packet);

			// 그녀셕이 이동중일경우 이동패킷 보냄
			switch (pExistCharacter->_action) {
			case dfACTION_MOVE_LL:
			case dfACTION_MOVE_LU:
			case dfACTION_MOVE_UU:
			case dfACTION_MOVE_RU:
			case dfACTION_MOVE_RR:
			case dfACTION_MOVE_RD:
			case dfACTION_MOVE_DD:
			case dfACTION_MOVE_LD:
				_LOG(dfLOG_LEVEL_DEBUG, L"ChatacterSectorUpdatePacket 4) CHARACTER_MOVE\n\tID[%d] world pos[%d, %d], action[%d], movedir [%d] dir[%d]", pExistCharacter->_SID, pExistCharacter->_X, pExistCharacter->_Y, pExistCharacter->_action, pExistCharacter->_moveDirection, pExistCharacter->_direction);
				MakePacket_Move_Start(&header, &packet,
					pExistCharacter->_SID,
					(BYTE)pExistCharacter->_action,
					pExistCharacter->_X,
					pExistCharacter->_Y);

				SendUnicast(pCharacter->_pSession, &header, &packet);
				break;
			}

		}

	}

}