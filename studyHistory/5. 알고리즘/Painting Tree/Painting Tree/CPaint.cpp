#include "CPaint.h"

#define dfRADIUS 20

CPaint::CPaint() {
	_hWnd = NULL;

	_hLinePen = (HPEN) GetStockObject(BLACK_PEN);
	_hNodePen = (HPEN) GetStockObject(BLACK_PEN);

	_hRedNodeBrush = CreateSolidBrush(RGB(255, 0, 0));
	_hBlackNodeBrush = CreateSolidBrush(RGB(0, 0, 0));

}

CPaint::~CPaint() {

}

void CPaint::SetHWND(HWND hWnd) {
	_hWnd = hWnd;
}

void CPaint::PrintLine(int sX, int sY, int dX, int dY) {
	HDC hdc = GetDC(_hWnd);
	SelectObject(hdc, _hNodePen);

	// 직선 선그리기
	MoveToEx(hdc, sX, sY, NULL);	// 시작점
	LineTo(hdc, dX, dY);			// 끝점

	// DC연결 해지
	ReleaseDC(_hWnd, hdc);
}


void CPaint::PrintNode(int iX, int iY, int iD, int iData, BYTE byColor) {
	WCHAR DataText[30];
	wsprintf(DataText, L"%4d", iData); // 4칸짜리 데이터 포멧

	HDC hdc = GetDC(_hWnd);

	// 원 위치 + 크기 계산
	RECT circle;
	circle.top = iY ;
	circle.left = iX ;
	circle.bottom = iY + iD;
	circle.right = iX + iD;


	// 팬선택
	SelectObject(hdc, _hNodePen);
	// 채울 색 선택
	if (byColor == 0) {
		// 블랙노드
		SelectObject(hdc, _hBlackNodeBrush);
	} else {
		// 레드노드
		SelectObject(hdc, _hRedNodeBrush);
	}

	// 원그리고
	Ellipse(hdc, circle.left, circle.top, circle.right, circle.bottom);
	// 택스트 출력
	TextOutW(hdc, iX , iY+(iD/3), DataText, wcslen(DataText));
	// DC연결 해지
	ReleaseDC(_hWnd, hdc);
}


