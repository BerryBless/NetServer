#include "DrawMap.h"
#include"CFramework.h"
#include <stdlib.h>

extern short g_map[dfMAPHEIGHT][dfMAPWIDTH];
CDisplayMap::CDisplayMap() {
	_hWnd = NULL;

	// 팬 설정
	_hRectPen = (HPEN) CreatePen(PS_SOLID, 0, RGB(125, 125, 125));
	_hLinePen = (HPEN) CreatePen(PS_SOLID, 6, RGB(0,255,255));
	_hInfoPen = (HPEN) CreatePen(PS_SOLID, 0, RGB(200, 200, 0));

	// 브러쉬 설정
	_hBackBrush = (HBRUSH) CreateSolidBrush(RGB(0, 0, 0));			// 빈 사각형
	_hWallBrush = (HBRUSH) CreateSolidBrush(RGB(192, 181, 171));	// 벽
	_hStartBrush = (HBRUSH) CreateSolidBrush(RGB(82, 210, 152));	// 시작지점
	_hEndBrush = (HBRUSH) CreateSolidBrush(RGB(210, 82, 140));		// 끝지점
	_hOpenBrush = (HBRUSH) CreateSolidBrush(RGB(255, 0, 200));	// 예약지점
	_hCloseBrush = (HBRUSH) CreateSolidBrush(RGB(255, 248, 0));	// 완료지점


	_half = (dfRectLen / 2);

}

CDisplayMap::~CDisplayMap() {
}

void CDisplayMap::Init(HWND hWnd) {
	_hWnd = hWnd;
	_half = (dfRectLen / 2);
	CreateMemDC();
}

void CDisplayMap::SetHWND(HWND hWnd) {
	_hWnd = hWnd;
	CreateMemDC();
}


void CDisplayMap::Display() {

	RECT rect;
	GetClientRect(_hWnd, &rect);                                 // 윈도우 크기 얻어오기
	HBRUSH hOldBrush = (HBRUSH) SelectObject(_hMemDC, _hBackBrush);
	PatBlt(_hMemDC, 0, 0, rect.right, rect.bottom, PATCOPY);  // 버퍼 다지우기 (브러쉬색 사각형으로 만듦)
	SelectObject(_hMemDC, hOldBrush);


	for (int i = 0; i < dfMAPHEIGHT; i++) {
		for (int j = 0; j < dfMAPWIDTH; j++) {
			DrawRect(i, j);
		}
	}

}

void CDisplayMap::DrawRect(int i, int j) {
	HBRUSH oldBrush;
	HBRUSH randBrush;
	RECT rect;
	rect.top = i * dfRectLen;	// 위
	rect.left = j * dfRectLen;	// 왼쪽
	rect.bottom = rect.top + dfRectLen + 1; // 아래
	rect.right = rect.left + dfRectLen + 1; // 오른쪽


	SelectObject(_hMemDC, _hRectPen);		// 사각형 팬
	randBrush = (HBRUSH) CreateSolidBrush(RGB((g_map[i][j] * 10) % 155+100, (g_map[i][j] * 20) % 155 + 100, (g_map[i][j] * 30) % 155 + 100));

	switch (g_map[i][j]) {
	case (short) eMAP::Empty:
		oldBrush = (HBRUSH) SelectObject(_hMemDC, GetStockObject(WHITE_BRUSH));
		break;
	case (short)eMAP::Wall:// 벽
		oldBrush = (HBRUSH) SelectObject(_hMemDC, _hWallBrush);
		break;
	case (short) eMAP::Start://시작지점
		oldBrush = (HBRUSH) SelectObject(_hMemDC, _hStartBrush);
		break;
	case (short) eMAP::End://끝지점
		oldBrush = (HBRUSH) SelectObject(_hMemDC, _hEndBrush);
		break;
	case (short) eMAP::Open:// 예약지점
		oldBrush = (HBRUSH) SelectObject(_hMemDC, _hOpenBrush);
		break;
	case (short) eMAP::Close://방문지점
		oldBrush = (HBRUSH) SelectObject(_hMemDC, _hCloseBrush);
		break;
	default:// 비어있음
		oldBrush = (HBRUSH) SelectObject(_hMemDC, randBrush);
		break;
	}

	// 사각형그리기
	Rectangle(_hMemDC, rect.left, rect.top, rect.right, rect.bottom);

	if (g_map[i][j] > (short) eMAP::MAX) {
		// 랜덤브러쉬 할당해지
		SelectObject(_hMemDC, oldBrush);
	}
		DeleteObject(randBrush);

	// 해당 노드 정보 가져오기
	stPATH *pPathNode = CFramework::GetInstance()->GetPositionInfo(j, i);
	// 없음 리턴
	if (pPathNode == NULL) return;


#ifdef dfPRINTINFO
	// 칸 정보 텍스트 출력
	WCHAR infoString[30];
	SelectObject(_hMemDC, _hInfoPen);		// 정보 팬
	wsprintfW(infoString, L"G = %d\nH = %d\nF = %d", pPathNode->G, pPathNode->F - pPathNode->G, pPathNode->F);
	DrawTextW(_hMemDC, infoString, -1, &rect, DT_CENTER);

#endif // dfPRINTINFO


	// 온 방향 그리기 (밖에서 중앙으로)
	if(pPathNode->pParent != NULL)
		Drawfrom(rect.left + _half, rect.top + _half,
			(pPathNode->pParent->X * dfRectLen) +_half, (pPathNode->pParent->Y * dfRectLen)+ _half,
			1);

}

void CDisplayMap::DrawPath(CList<stPOS> &path, HPEN pen) {
	HPEN oldpen;
	auto iter = path.begin();
	int sX, sY;// 시작
	int eX, eY;// 끝
	sX = (*iter).X * dfRectLen + _half;
	sY = (*iter).Y * dfRectLen + _half;
	++iter;
	oldpen = (HPEN)SelectObject(_hMemDC, pen);		// 경로 팬

	while (iter != path.end()) {
		eX = (*iter).X * dfRectLen + _half;
		eY = (*iter).Y * dfRectLen + _half;

		MoveToEx(_hMemDC, sX, sY, NULL);
		LineTo(_hMemDC, eX, eY);

		sX = eX;
		sY = eY;
		++iter;
	}
	SelectObject(_hMemDC, oldpen);		// 경로팬

}

void CDisplayMap::Drawfrom(int pX, int pY, int vX, int vY, int len) {
	int eX = (vX * len);
	int eY = (vY * len);

	SelectObject(_hMemDC, _hInfoPen);

	MoveToEx(_hMemDC, pX, pY, NULL);
	LineTo(_hMemDC, eX, eY);

}

void CDisplayMap::DrawLine(int sX, int sY, int eX, int eY) {
	SelectObject(_hMemDC, _hInfoPen);

	MoveToEx(_hMemDC, sX, sY, NULL);
	LineTo(_hMemDC, eX, eY);
}

void CDisplayMap::DrawUI() {
	// 칸 정보 텍스트 출력
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = 500;
	rect.bottom = 500;
	const WCHAR infoString[] = L"벽생성 : 드래그\n시작지점 변경 : 왼쪽 더블클릭\n끝지점 변경 : 오른쪽 더블클릭\nJSP실행 : 휠버튼 클릭\n 맵생성 : M\n지우기 : 스페이스바";
	SelectObject(_hMemDC, _hInfoPen);		// 정보 팬
	DrawTextW(_hMemDC, infoString, -1, &rect, DT_CENTER);
}





void CDisplayMap::CreateMemDC() {
	//-------------------------------------
// 메모리DC 생성 부분
//-------------------------------------
// 화면 크기 구하기
	RECT Rect;
	GetClientRect(_hWnd, &Rect);

	// 백버퍼 생성
	HDC hdc = GetDC(_hWnd);


	_hMemDC = CreateCompatibleDC(hdc);
	_hMemBitmap = CreateCompatibleBitmap(hdc, Rect.right, Rect.bottom);
	_hMemBitmapOld = (HBITMAP) SelectObject(_hMemDC, _hMemBitmap);
	ReleaseDC(_hWnd, hdc);

	PatBlt(_hMemDC, 0, 0, Rect.right, Rect.bottom, BLACKNESS);

	SetBkMode(_hMemDC, TRANSPARENT); // 글자 뒤 배경 투명하게
	SetROP2(_hMemDC, R2_XORPEN);	// 겹치는 부분 xor
	// TODO 폰트 xor
	// 윈도우 크기정보 저장
	_iWindowPosX = Rect.left;
	_iWindowPosY = Rect.top;
	_iWindowWidth = Rect.right - Rect.left;
	_iWindowHeight = Rect.bottom - Rect.top;
}

void CDisplayMap::FlipMemDC() {
	// 버퍼 복사하기
	RECT rect;
	HDC hDC;

	// 플립
	GetWindowRect(_hWnd, &rect);
	hDC = GetDC(_hWnd);

	BitBlt(hDC, 0, 0, rect.right, rect.bottom, _hMemDC, 0, 0, SRCCOPY);


	ReleaseDC(_hWnd, hDC);
}

void CDisplayMap::DestroyMemDC() {
	//-------------------------------------
	// 메모리DC 파괴 부분
	//-------------------------------------
	if (_hMemBitmapOld != NULL)
		SelectObject(_hMemDC, _hMemBitmapOld);
	DeleteObject(_hMemBitmap);
	DeleteObject(_hMemDC);
}

void CDisplayMap::Resize() {
	DestroyMemDC();
	CreateMemDC();
}
