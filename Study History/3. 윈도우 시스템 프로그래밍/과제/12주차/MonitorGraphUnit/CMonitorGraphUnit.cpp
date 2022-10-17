#include "CMonitorGraphUnit.h"
#include "framework.h"
#include "MonitorGraphUnit.h"
// TEMP
int CMonitorGraphUnit::_childCnt = 0;
CMonitorGraphUnit::stHWNDtoTHIS CMonitorGraphUnit::_hWndToThis;

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
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
//------------------------------------------------------
// 윈도우 핸들, this 포인터 매칭 테이블 관리.
//------------------------------------------------------
CMonitorGraphUnit *CMonitorGraphUnit::GetThis(HWND hWnd) {
	for (int i = 0; i < dfMAXCHILD; i++) {
		if (hWnd == CMonitorGraphUnit::_hWndToThis.hWnd[i])
			return CMonitorGraphUnit::_hWndToThis.pThis[i];
	}

	return NULL;
}

#pragma endregion

CMonitorGraphUnit::CMonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight) {
	// TODO 클래스 이름 정하기
	// TEMP 일단 넘버링으로
	WCHAR childName[30];
	wsprintf(childName, L"ChildClass_%d", CMonitorGraphUnit::_childCnt);

	// 자식클래스 WndProc등록
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CMonitorGraphUnit::WndProc;
	wcex.cbClsExtra = 0;// 클래스 공간에 바이트 확보, 클래스 끼리 공유가능?
	wcex.cbWndExtra = 0;// 윈도우 공간에 바이트 확보
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MONITORGRAPHUNIT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_MONITORGRAPHUNIT);
	wcex.lpszClassName = childName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassExW(&wcex);


	// 자식클래스 인스턴스 생성
	_hWnd = CreateWindow(childName, NULL, WS_CHILD | WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS,
		iPosX, iPosY, iWidth, iHeight,
		hWndParent, NULL, hInstance, NULL);

	// 생성실패
	if (!_hWnd) {
		// TODO 에러처리
		int *p = nullptr;
		*p = 10;
	}
	// 타입지정
	_enGraphType = enType;

	// 더블버퍼링 버퍼 생성
	CreateMemDC();

	// This포인터 저장
	PutThis();
}
CMonitorGraphUnit::~CMonitorGraphUnit() {
	DestroyMemDC();
}

BOOL CMonitorGraphUnit::InsertData(int iData) {
	// 창 크기에 따른 데이터의 최대 개수
	RECT rect;
	GetClientRect(_hWnd, &rect);
	int max_X = (int) ((float) (rect.right - rect.left) * (1.0f / 4.0f));

	// 데이터 넣기
	if (_dataList.size() > max_X) {
		_dataList.pop_front();
	}
	_dataList.push_back(iData);

	DrawMemDC();

	return true;
}
//------------------------------------------------------
// 윈도우 핸들, this 포인터 매칭 테이블 관리.
//------------------------------------------------------
BOOL CMonitorGraphUnit::PutThis(void) {
	CMonitorGraphUnit::_hWndToThis.hWnd[CMonitorGraphUnit::_childCnt] = _hWnd;
	CMonitorGraphUnit::_hWndToThis.pThis[CMonitorGraphUnit::_childCnt] = this;
	++CMonitorGraphUnit::_childCnt;
	return true;
}

void CMonitorGraphUnit::CreateMemDC() {
	//-------------------------------------
// 메모리DC 생성 부분
//-------------------------------------
	RECT Rect;
	GetClientRect(_hWnd, &Rect);

	HDC hdc = GetDC(_hWnd);
	_hMemDC = CreateCompatibleDC(hdc);
	_hMemBitmap = CreateCompatibleBitmap(hdc, Rect.right, Rect.bottom);
	_hMemBitmapOld = (HBITMAP) SelectObject(_hMemDC, _hMemBitmap);
	ReleaseDC(_hWnd, hdc);

	PatBlt(_hMemDC, 0, 0, Rect.right, Rect.bottom, WHITENESS);
}
void CMonitorGraphUnit::DrawMemDC() {
	//그림 그리기
	// 그릴공간세팅
	PAINTSTRUCT  ps;
	RECT rect;
	HDC hdc;
	int dataX = 0; // 데이터를 그릴 x좌표(시간)

	GetClientRect(_hWnd, &rect);                                 // 윈도우 크기 얻어오기
	PatBlt(_hMemDC, 0, 0, rect.right, rect.bottom, WHITENESS);  // 버퍼 다지우기 (하얀색 사각형으로 만듦)
	// ---------------
	// 그림 그리기
	// ---------------

	switch (_enGraphType) {
	case TYPE::LINE_SINGLE:
		// 선하나
	{
		auto iter = _dataList.begin();
		MoveToEx(_hMemDC, dataX, *iter, NULL);
		while (iter != _dataList.end()) {
			LineTo(_hMemDC, dataX, *iter);
			dataX += 3;
			++iter;
		}
	}
		break;
	default:
		break;
	}



	// 갱신(플립)
	InvalidateRect(_hWnd, NULL, FALSE);
}
void CMonitorGraphUnit::FlipMemDC() {
	// 버퍼 복사하기
	PAINTSTRUCT  ps;
	RECT rect;
	HDC hdc;

	// 플립
	GetClientRect(_hWnd, &rect);
	hdc = BeginPaint(_hWnd, &ps);
	BitBlt(hdc, 0, 0, rect.right, rect.bottom, _hMemDC, 0, 0, SRCCOPY);
	EndPaint(_hWnd, &ps);
}
void CMonitorGraphUnit::DestroyMemDC() {
	//-------------------------------------
	// 메모리DC 파괴 부분
	//-------------------------------------
	SelectObject(_hMemDC, _hMemBitmapOld);
	DeleteObject(_hMemBitmap);
	DeleteObject(_hMemDC);
}


