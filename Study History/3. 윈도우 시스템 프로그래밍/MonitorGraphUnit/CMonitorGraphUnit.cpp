#include "CMonitorGraphUnit.h"
#include "MonitorGraphUnit.h"
#include "framework.h"


// TEMP
int CMonitorGraphUnit::_childCnt = 0;

#pragma region static method


LRESULT CALLBACK  CMonitorGraphUnit::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	CMonitorGraphUnit *pThis = CMonitorGraphUnit::GetThis(hWnd);
	switch (message) {
	case WM_CREATE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		// 화면 갱신할
		if (pThis != NULL) {
			switch (pThis->_enGraphType) {
			case CMonitorGraphUnit::TYPE::LINE_SINGLE:
				pThis->Paint_LineSingle();
				break;
			}
			pThis->FlipMemDC();
		}
		break;
	case WM_SIZE:
		// 사이즈 크기 바뀌면
		if (pThis != NULL) {
			pThis->DestroyMemDC();
			pThis->CreateMemDC();
		}
		break;
	case WM_TIMER:
		pThis->ChangeBackgroundBrush();
		KillTimer(hWnd, 1);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return NULL;
}
//------------------------------------------------------
// 윈도우 핸들, this 포인터 매칭 테이블 관리.
//------------------------------------------------------
CMonitorGraphUnit *CMonitorGraphUnit::GetThis(HWND hWnd) {
	return (CMonitorGraphUnit *) GetWindowLongPtr(hWnd, 0);
}

#pragma endregion

CMonitorGraphUnit::CMonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, const WCHAR *szTitle, int iPosX, int iPosY, int iWidth, int iHeight, int iDataMax, int iDataAlert) {
	// 클래스 이름 복사
	wsprintf(_szTitle, L"%s", szTitle);

	// 자식클래스 WndProc등록
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CMonitorGraphUnit::WndProc;
	wcex.cbClsExtra = 8;// 클래스 공간에 바이트 확보, 클래스 끼리 공유가능?
	wcex.cbWndExtra = 8;// 윈도우 공간에 바이트 확보
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MONITORGRAPHUNIT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_MONITORGRAPHUNIT);
	wcex.lpszClassName = _szTitle;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassExW(&wcex);


	// 자식클래스 인스턴스 생성
	_hWnd = CreateWindow(_szTitle, NULL, WS_CHILD | WS_VISIBLE,
		iPosX, iPosY, iWidth, iHeight,
		hWndParent, NULL, hInstance, NULL);

	// 인스턴스 생성실패
	if (!_hWnd) {
		// TODO 에러처리
		int *p = nullptr;
		*p = 10;
	}
	// 부모 핸들 저장
	_hWndParent = hWndParent;
	_hInstance = hInstance;

	// 타입지정
	_enGraphType = enType;

	// This포인터 저장
	PutThis();

	// 데이터 최대값 지정
	_iDataMax = iDataMax;
	_iDataAlert = iDataAlert;
#pragma region Create Resource
	//------------------------------------------------------
	// 더블버퍼링 버퍼 생성
	//------------------------------------------------------
	CreateMemDC();

	//------------------------------------------------------
	// Font 생성.
	//------------------------------------------------------
	_hFontTitle = CreateFont(dfFONT_TITLE_SIZE, 0, 0, 0, dfFONT_TITLE_WIDTH, 0, 0, 0, DEFAULT_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, L"맑은 고딕");
	_hFontGrid = CreateFont(dfFONT_POINT_SIZE, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, L"맑은 고딕");

	//------------------------------------------------------
	// 백 브러쉬 생성  (색상 채우기용)
	//------------------------------------------------------
	 _BackColor = RGB(25, 25, 25);
	 _TitleColor = RGB(100, 100, 100);
	 _UIColor = RGB(0, 255, 0);

	_hBackBrush = CreateSolidBrush(_BackColor);
	_hBackBrush_Title = CreateSolidBrush(_TitleColor);


	// 각 자식 윈도우의 색상은  1개로 통일 하셔도 됩니다.
//  저도 처음에는 RGB 값을 생성자 인자로 받아서 사용 하였으나  사용이 너무 불편하였으며
//  그리고 4가지 정도의 색상을 고정적으로 쓰고 있습니다. 
// 실제로 써보니 늘 쓰던 진한 먹색 정도만 늘상 사용하게 됩니다.

	//------------------------------------------------------
	// 펜 생성
	//------------------------------------------------------
	BYTE byR = min(255, GetRValue(_UIColor) + 200);
	BYTE byG = min(255, GetGValue(_UIColor) + 200);
	BYTE byB = min(255, GetBValue(_UIColor) + 200);

	_hLinePen = CreatePen(PS_SOLID, 2, RGB(byR, byG, byB));

	// 백 화면보다  많이 밝은 색을 표현하기 위해  RGB 각 요소에 + 200 을 함, 거의 흰색에 가깝겠음

	byR = min(255, GetRValue(_UIColor) + 50);
	byG = min(255, GetGValue(_UIColor) + 50);
	byB = min(255, GetBValue(_UIColor) + 50);

	_hGridPen = CreatePen(PS_SOLID, 1, RGB(byR, byG, byB));

#pragma endregion

	++CMonitorGraphUnit::_childCnt;
}
CMonitorGraphUnit::~CMonitorGraphUnit() {
	DestroyMemDC();
}
//------------------------------------------------------
// 윈도우 핸들, this 포인터 매칭 테이블 관리.
//------------------------------------------------------
BOOL CMonitorGraphUnit::PutThis(void) {
	LONG_PTR p;
	p = SetWindowLongPtr(_hWnd, 0, (LONG_PTR) this);
	return p != NULL;
}

// ---------
// 데이터넣기
// ---------
BOOL CMonitorGraphUnit::InsertData(int iData) {
	// 창 크기에 따른 데이터의 최대 개수
	// 너비의 1/4 만 넣고, x간 간격이 3
	int max_X = (int) ((float) (_iWindowWidth) * (1.0f / 4.0f));

	// 데이터 넣기
	if (_dataList.size() > max_X) {
		_dataList.pop_front();
	}
	_dataList.push_back(iData);

	if (_iDataAlert != 0 && _iDataAlert <= iData) {
		//  경고 1회 !
		PlaySound((LPCTSTR) SND_ALIAS_SYSTEMHAND, NULL, SND_ALIAS_ID | SND_ASYNC); // 소리 출력
		DeleteObject(_hBackBrush);
		_hBackBrush = CreateSolidBrush(RGB(200,0,0));
		SetTimer(_hWnd, 1, dfALERT_TIME_MS, NULL);
	}

	// 화면갱신
	InvalidateRect(_hWnd, NULL, FALSE);
	return true;
}


#pragma region PaintMethod

void CMonitorGraphUnit::Paint_LineSingle() {
	//그림 그리기
	// 그릴공간세팅
	RECT rect;
	int dataX = 0; // 데이터를 그릴 x좌표(시간)
	HPEN	hOldPen = (HPEN) SelectObject(_hMemDC, _hLinePen);

	GetClientRect(_hWnd, &rect);                                 // 윈도우 크기 얻어오기
	HBRUSH hOldBrush = (HBRUSH) SelectObject(_hMemDC, _hBackBrush);
	PatBlt(_hMemDC, 0, 0, rect.right, rect.bottom, PATCOPY);  // 버퍼 다지우기 (브러쉬색 사각형으로 만듦)
	SelectObject(_hMemDC, hOldBrush);
	// ---------------
	// 그림 그리기
	// 1. 큐에서 데이터 뽑아 선긋기
	// 2. 타이틀바 출력
	// 3. 그리드 출력
	// ---------------

	// 1. 큐에서 데이터 뽑아 선긋기
	auto iter = _dataList.begin();
	MoveToEx(_hMemDC, dataX, GetDataYPos(*iter), NULL);
	while (iter != _dataList.end()) {

		LineTo(_hMemDC, dataX, GetDataYPos(*iter));
		dataX += 3;
		++iter;
	}

	SelectObject(_hMemDC, hOldPen);


	// 2. 타이틀바 출력
	Paint_Title();
	// 3. 그리드 출력
	Paint_Grid();

}
#pragma region UI
void CMonitorGraphUnit::Paint_Title() {
	BYTE byR = min(255, GetRValue(_BackColor) + 170);
	BYTE byG = min(255, GetGValue(_BackColor) + 170);
	BYTE byB = min(255, GetBValue(_BackColor) + 170);

	SetTextColor(_hMemDC, RGB(byR, byG, byB));

	// 이는 DC 에 텍스트 출력시 글자의 색상을 지정합니다.
	// 백 색상보다 밝은 색상을 지정 했습니다.


	HPEN	hPen = CreatePen(PS_NULL, 0, 0);
	HPEN	hOldPen = (HPEN) SelectObject(_hMemDC, hPen);

	//..만들어둔 부러시도 SelectObject 해야 함 ..
	HBRUSH hOldBrush = (HBRUSH) SelectObject(_hMemDC, _hBackBrush_Title);

	RECT rect;
	GetClientRect(_hWnd, &rect);                                 // 윈도우 크기 얻어오기


	Rectangle(_hMemDC, 0, 0, _iWindowWidth + 1, dfTITLE_RECT_HEIGHT);

	// Ractangle 사각형 그리기로  상단 부분에 30pixel 높이로 사각형을 그립니다.
	// Ractangle 출력시 외곽선은 Pen, 내부는 Brush 로 채워집니다.

	//  외곽선을 없애기 위해서 CreatePen(PS_NULL  로 Null 펜을 만들어서 지정 하였습니다.
	//  이 nulll 펜도 하나 만들어 두고 재사용 하셔도 됩니다



	SelectObject(_hMemDC, hOldPen);
	SelectObject(_hMemDC, hOldBrush);
	DeleteObject(hPen);


	HFONT hOldFont = (HFONT) SelectObject(_hMemDC, _hFontTitle);
	TextOut(_hMemDC, 7, 3, _szTitle, (int) wcslen(_szTitle));
	SelectObject(_hMemDC, hOldFont);

}
void CMonitorGraphUnit::Paint_Grid() {
	HPEN	hOldPen = (HPEN) SelectObject(_hMemDC, _hGridPen);

	int iDataInterval = _iDataMax / 4;		// Max 데이터를 기준으로 4등분 해보았습니다.  
						// Max 데이터를 기준으로 4등분이 아닌,  그냥 윈도우 크기 기준으로 4등분을 하면 안되느냐 ? 라고 하실 수 있으나
						//  4등분 그리드 선에  실제 데이터 수치도 표기를 해주어야 하므로  실제 데이터값 기준으로 구한 것입니다.


	int iY = _iWindowPosY + dfTITLE_RECT_HEIGHT;


	int iIntervalY = (_iWindowHeight - dfTITLE_RECT_HEIGHT) / 4;	// 실제 화면 윈도우 크기기준 4등분


	HFONT hOldFont = (HFONT) SelectObject(_hMemDC, _hFontGrid);

	WCHAR szPoint[10];
	wsprintf(szPoint, L"%d", _iDataMax);
	//----------------------------------------------------
	// 각 위치별 수치 찍기
	//----------------------------------------------------

	TextOut(_hMemDC, _iWindowPosX + 1, iY + 1, szPoint, (int) wcslen(szPoint));	// 가장 상단부의 수치를 찍어봅니다.  수치가 szPoint 에 구해져 있습니다.  

	for (int iCnt = 3; iCnt > 0; --iCnt) {
		iY += iIntervalY;

		//----------------------------------------------------
		// 세로축 선 긋기
		//----------------------------------------------------
		MoveToEx(_hMemDC, _iWindowPosX + 1, iY + 1, NULL);
		LineTo(_hMemDC, _iWindowPosX + _iWindowWidth - 1, iY + 1);

		//----------------------------------------------------
		// 각 위치별 수치 찍기
		//----------------------------------------------------
		wsprintf(szPoint, L"%d", _iDataMax / (4) * (iCnt));
		TextOut(_hMemDC, _iWindowPosX + 1, iY + 1, szPoint, (int) wcslen(szPoint));
	}

	SelectObject(_hMemDC, hOldFont);
	SelectObject(_hMemDC, hOldPen);
}
#pragma endregion

int CMonitorGraphUnit::GetDataYPos(int data) {
	// 그래프로 대략 표현하기
	// min( bottom-1 ,  bottom - (Height * (data/dataMax) )
	return min((_iWindowHeight + _iWindowPosY - 1), (_iWindowHeight + _iWindowPosY) -
		(int) ((double) (_iWindowHeight - dfTITLE_RECT_HEIGHT) * ((double) data / (double) _iDataMax)));
}

// 
void CMonitorGraphUnit::ChangeBackgroundBrush() {
	DeleteObject(_hBackBrush);
	_hBackBrush = CreateSolidBrush(_BackColor);
}



#pragma endregion










#pragma region MemDC
void CMonitorGraphUnit::CreateMemDC() {
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
	SetROP2(_hMemDC, R2_XORPEN);	// 그래프 그리드랑 xor
	// TODO 폰트 xor
	// 윈도우 크기정보 저장
	_iWindowPosX = Rect.left;
	_iWindowPosY = Rect.top;
	_iWindowWidth = Rect.right - Rect.left;
	_iWindowHeight = Rect.bottom - Rect.top;
}

void CMonitorGraphUnit::FlipMemDC() {
	// 버퍼 복사하기
	PAINTSTRUCT  ps;
	RECT rect;
	HDC hdc;

	// 플립
	GetClientRect(_hWnd, &rect);
	hdc = BeginPaint(_hWnd, &ps);
	
	BitBlt(hdc, 0, 0, rect.right, rect.bottom, _hMemDC, 0, 0, SRCCOPY );


	EndPaint(_hWnd, &ps);
}

void CMonitorGraphUnit::DestroyMemDC() {
	//-------------------------------------
	// 메모리DC 파괴 부분
	//-------------------------------------
	if (_hMemBitmapOld != NULL)
		SelectObject(_hMemDC, _hMemBitmapOld);
	DeleteObject(_hMemBitmap);
	DeleteObject(_hMemDC);
}
#pragma endregion


