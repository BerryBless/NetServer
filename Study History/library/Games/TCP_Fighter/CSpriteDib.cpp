#include "CSpriteDib.h"

CSpriteDib::CSpriteDib(int iMaxSprite, DWORD dwColorKey) {
	_iMaxSprite = iMaxSprite;
	_dwColorKey = dwColorKey;

	_stpSprite = NULL;

	// 최대크기만큼 미리 동적할당
	_stpSprite = new stSPRITE[_iMaxSprite];
	memset(_stpSprite, 0, sizeof(stSPRITE) * _iMaxSprite);

}

CSpriteDib::~CSpriteDib() {
	for (int i = 0; i < _iMaxSprite; i++) {
		ReleaseSprite(i);
	}
}

BOOL CSpriteDib::LoadDibSprite(int iSpriteIndex, const WCHAR *szFileName, int iCenterPointX, int iCenterPointY) {
	HANDLE hFile;
	DWORD dwRead;
	int iPitch;
	int iImageSize;
	BITMAPFILEHEADER stFileHeader;
	BITMAPINFOHEADER stInfoHeader;


	// 파일열기
	hFile = CreateFile(szFileName, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	ReleaseSprite(iSpriteIndex);
	ReadFile(hFile, &stFileHeader, sizeof(BITMAPFILEHEADER), &dwRead, NULL);

	if (stFileHeader.bfType == 0x4d42) {
		// 인포헤더를 읽어서 저장, 32비트확인
		ReadFile(hFile, &stInfoHeader, sizeof(BITMAPINFOHEADER), &dwRead, NULL);
		if (stInfoHeader.biBitCount == 32) {
			// 한줄 한줄 피치값 구하기
			iPitch = (stInfoHeader.biWidth * 4) + 3 & ~3;

			// 스프라이트 구조체에 크기 저장
			_stpSprite[iSpriteIndex].iWidth = stInfoHeader.biWidth;
			_stpSprite[iSpriteIndex].iHeight = stInfoHeader.biHeight;
			_stpSprite[iSpriteIndex].iPitch = iPitch;

			// 이미지에 대한 전체 크기를 구하고 메모리 할당
			iImageSize = iPitch * stInfoHeader.biHeight;
			_stpSprite[iSpriteIndex].bypImage = new BYTE[iImageSize];

			// 이미지 부분은 스프라이트 버퍼로 읽어온다
			// DIB는 뒤집어져 있으므로 다시 뒤집자
			// 임시 버퍼에 읽은 뒤 뒤집으면서 복사한다.
			BYTE *bypTempBuffer = new BYTE[iImageSize];
			BYTE *bypSpriteTemp = _stpSprite[iSpriteIndex].bypImage;
			BYTE *bypTurnTemp;

			ReadFile(hFile, bypTempBuffer, iImageSize, &dwRead, NULL);

			// 한줄 한줄 뒤집자
			bypTurnTemp = bypTempBuffer + iPitch * (stInfoHeader.biHeight - 1);

			for (int i = 0; i < stInfoHeader.biHeight; i++) {
				memcpy(bypSpriteTemp, bypTurnTemp, iPitch);
				bypSpriteTemp += iPitch;
				bypTurnTemp -= iPitch;
			}
			delete[] bypTempBuffer;

			_stpSprite[iSpriteIndex].iCenterPointX = iCenterPointX;
			_stpSprite[iSpriteIndex].iCenterPointY = iCenterPointY;

			CloseHandle(hFile);
			return TRUE;
		}
	}
	CloseHandle(hFile);
	return FALSE;
}

BOOL CSpriteDib::LoadDibSprite(e_SPRITE eSpriteIndex, const WCHAR *szFileName, int iCenterPointX, int iCenterPointY) {
	return LoadDibSprite((int) eSpriteIndex, szFileName, iCenterPointX, iCenterPointY);
}

void CSpriteDib::ReleaseSprite(int iSpriteIndex) {
	if (_iMaxSprite <= iSpriteIndex) {
		// 잘못된 인덱스
		return;
	}

	if (_stpSprite[iSpriteIndex].bypImage != NULL) {
		// 삭제후 초기화
		delete[] _stpSprite[iSpriteIndex].bypImage;
		memset(&_stpSprite[iSpriteIndex], 0, sizeof(stSPRITE));
	}
}

void CSpriteDib::DrawSprite(int iSpriteIndex, int iDrawX, int iDrawY, BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch, int iDrawLen, bool bColorKey) {
	// 최대 스프라이트 개수를 초과하거나 로드되지 않은 스프라이트면 무시
	if (iSpriteIndex >= _iMaxSprite)return;
	if (_stpSprite[iSpriteIndex].bypImage == NULL) return;

	// 시작
	stSPRITE *stpSprite = &_stpSprite[iSpriteIndex];

	// 스프라이트 출력을 위해 사이즈 저장
	int iSpriteWidth = stpSprite->iWidth;
	int iSpriteHeight = stpSprite->iHeight;

	// 출력길이 설정
	iSpriteWidth = iSpriteWidth * iDrawLen / 100;

	DWORD *dwpDest = (DWORD *) bypDest;
	DWORD *dwpSprite = (DWORD *) (stpSprite->bypImage);

	// 출력 중점 처리
	iDrawX = iDrawX - stpSprite->iCenterPointX;
	iDrawY = iDrawY - stpSprite->iCenterPointY;

#pragma region Clipping
	// 상단 (위쪽이 짤림)
	if (0 > iDrawY) {
		iSpriteHeight = iSpriteHeight - (-iDrawY);
		dwpSprite = (DWORD *) (stpSprite->bypImage + stpSprite->iPitch * (-iDrawY));

		iDrawY = 0;
	}

	// 하단 (아래쪽이 짤림)
	if (iDestHeight < iDrawY + stpSprite->iHeight) {
		iSpriteHeight -= ((iDrawY + stpSprite->iHeight) - iDestHeight);
	}

	// 좌측 (왼쪽이 짤림)
	if (0 > iDrawX) {
		iSpriteWidth = iSpriteWidth - (-iDrawX);
		dwpSprite = dwpSprite + (-iDrawX);

		iDrawX = 0;
	}
	// 우측 (오른쪽이 짤림)
	if (iDestWidth < iDrawX + stpSprite->iWidth) {
		iSpriteWidth -= ((iDrawX + stpSprite->iWidth) - iDestWidth);
	}

	// 찍을 그림이 없다면 종료
	if (iSpriteWidth <= 0 || iSpriteHeight <= 0)return;
#pragma endregion

	// 화면 찍기
	// 찍을 위치로 이동
	dwpDest = (DWORD *) (((BYTE *) (dwpDest + iDrawX) + (iDrawY * iDestPitch)));

	BYTE *bypDestOrigin = (BYTE *) dwpDest;
	BYTE *bypSpriteOrigin = (BYTE *) dwpSprite;

	// 전체 크기를 돌면서 투명색 처리를 한다.
	for (int y = 0; iSpriteHeight > y; y++) {
		for (int x = 0; iSpriteWidth > x; x++) {
			if (_dwColorKey != (*dwpSprite & 0x00FFFFFF) || bColorKey) {
				*dwpDest = *dwpSprite;
			}

			dwpDest++;
			dwpSprite++;
		}

		//다음줄로 이동
		bypDestOrigin = bypDestOrigin + iDestPitch;
		bypSpriteOrigin = bypSpriteOrigin + stpSprite->iPitch;

		dwpDest = (DWORD *) bypDestOrigin;
		dwpSprite = (DWORD *) bypSpriteOrigin;
	}

}

void CSpriteDib::DrawSprite(e_SPRITE eSpriteIndex, int iDrawX, int iDrawY, BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch, int iDrawLen, bool bColorKey) {
	DrawSprite((int) eSpriteIndex, iDrawX, iDrawY, bypDest, iDestWidth, iDestHeight, iDestPitch, iDrawLen, bColorKey);
}

void CSpriteDib::DrawSpriteRed(int iSpriteIndex, int iDrawX, int iDrawY, BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch) {
	// 최대 스프라이트 개수를 초과하거나 로드되지 않은 스프라이트면 무시
	if (iSpriteIndex >= _iMaxSprite)return;
	if (_stpSprite[iSpriteIndex].bypImage == NULL) return;

	// 시작
	stSPRITE *stpSprite = &_stpSprite[iSpriteIndex];

	// 스프라이트 출력을 위해 사이즈 저장
	int iSpriteWidth = stpSprite->iWidth;
	int iSpriteHeight = stpSprite->iHeight;


	DWORD *dwpDest = (DWORD *) bypDest;
	DWORD *dwpSprite = (DWORD *) (stpSprite->bypImage);

	// 출력 중점 처리
	iDrawX = iDrawX - stpSprite->iCenterPointX;
	iDrawY = iDrawY - stpSprite->iCenterPointY;

#pragma region Clipping
	// 상단 (위쪽이 짤림)
	if (0 > iDrawY) {
		iSpriteHeight = iSpriteHeight - (-iDrawY);
		dwpSprite = (DWORD *) (stpSprite->bypImage + stpSprite->iPitch * (-iDrawY));

		iDrawY = 0;
	}

	// 하단 (아래쪽이 짤림)
	if (iDestHeight < iDrawY + stpSprite->iHeight) {
		iSpriteHeight -= ((iDrawY + stpSprite->iHeight) - iDestHeight);
	}

	// 좌측 (왼쪽이 짤림)
	if (0 > iDrawX) {
		iSpriteWidth = iSpriteWidth - (-iDrawX);
		dwpSprite = dwpSprite + (-iDrawX);

		iDrawX = 0;
	}
	// 우측 (오른쪽이 짤림)
	if (iDestWidth < iDrawX + stpSprite->iWidth) {
		iSpriteWidth -= ((iDrawX + stpSprite->iWidth) - iDestWidth);
	}

	// 찍을 그림이 없다면 종료
	if (iSpriteWidth <= 0 || iSpriteHeight <= 0)return;
#pragma endregion

	// 화면 찍기
	// 찍을 위치로 이동
	dwpDest = (DWORD *) (((BYTE *) (dwpDest + iDrawX) + (iDrawY * iDestPitch)));

	BYTE *bypDestOrigin = (BYTE *) dwpDest;
	BYTE *bypSpriteOrigin = (BYTE *) dwpSprite;

	// 전체 크기를 돌면서 투명색 처리를 한다.
	for (int y = 0; iSpriteHeight > y; y++) {
		for (int x = 0; iSpriteWidth > x; x++) {
			if (_dwColorKey != (*dwpSprite & 0x00FFFFFF)) {
				// 색바꾸기
				*dwpDest = (*dwpSprite | RGB(0, 0, 255)) ;
			}
			// 다음찍을 픽셀
			dwpDest++;
			dwpSprite++;
		}

		//다음줄로 이동
		bypDestOrigin = bypDestOrigin + iDestPitch;
		bypSpriteOrigin = bypSpriteOrigin + stpSprite->iPitch;

		dwpDest = (DWORD *) bypDestOrigin;
		dwpSprite = (DWORD *) bypSpriteOrigin;
	}
}

BOOL CSpriteDib::GetSpriteHitbox(int iSpriteIndex, int iX, int iY, RECT *rpHitbox) {
	// 최대 스프라이트 개수를 초과하거나 로드되지 않은 스프라이트면 무시
	if (iSpriteIndex >= _iMaxSprite)return FALSE;
	if (_stpSprite[iSpriteIndex].bypImage == NULL) return FALSE;

	stSPRITE stSprite = _stpSprite[iSpriteIndex];
	// 히트박스 계산
	rpHitbox->left =iX - stSprite.iCenterPointX; // 왼쪽위 x
	rpHitbox->top = iY - stSprite.iCenterPointY;	// 왼쪽위 Y
	rpHitbox->right = rpHitbox->left + stSprite.iWidth; // 오른쪽아래 x
	rpHitbox->bottom = rpHitbox->top+ stSprite.iHeight; // 오른쪽아래 y

	// 클리핑처리
	rpHitbox->left = max(rpHitbox->left, 0);
	rpHitbox->top = max(rpHitbox->top, 0);
	rpHitbox->right = min(rpHitbox->right, 640);
	rpHitbox->bottom = min(rpHitbox->bottom, 480);

	return TRUE;
}

//void CSpriteDib::DrawImage(int iSpriteIndex, int iDrawX, int iDrawY, BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch, int iDrawLen) {
//}
