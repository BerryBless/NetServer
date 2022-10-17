// TimerGraph.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "TimerGraph.h"
#include "CQueue.h"

#define MAX_LOADSTRING 100
#define MAX_DATACOUNT 300	// 최대 데이터
#define X_AXISUNCREMENT 5	// 그릴때 x축 너비

// Global Variables:
HINSTANCE hInst;                                // current instance


// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);	// 타이머용
void CreateMemDC(HWND);
void DrawMemDC(HWND);
void DestroyMemDC();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIMERGRAPH));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TIMERGRAPH);
	wcex.lpszClassName = L"TimerGraph";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	// Perform application initialization:
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(L"TimerGraph", L"graph?", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);




	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TIMERGRAPH));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//-------------------------------------
// 전역에 메모리DC 핸들 보관
//-------------------------------------
HDC	_hMemDC;
HBITMAP	_hMemBitmap;
HBITMAP _hMemBitmapOld;

// Y 데이터 리스트
CList<int> _DataList;


// 타이머 프로시저
void CALLBACK   TimerProc(HWND hWnd, UINT message, UINT_PTR wParam, DWORD lParam) {
	// 데이터 추가
	
	if (_DataList.size() > MAX_DATACOUNT) {
		_DataList.pop_front();
	}

	int yData = rand() % 100 + 500;

	_DataList.push_back(yData);

	DrawMemDC(hWnd);
}

void CreateMemDC(HWND hWnd) {
	//-------------------------------------
	// 메모리DC 생성 부분
	//-------------------------------------
	RECT Rect;
	GetClientRect(hWnd, &Rect);

	HDC hdc = GetDC(hWnd);
	_hMemDC = CreateCompatibleDC(hdc);
	_hMemBitmap = CreateCompatibleBitmap(hdc, Rect.right, Rect.bottom);
	_hMemBitmapOld = (HBITMAP) SelectObject(_hMemDC, _hMemBitmap);
	ReleaseDC(hWnd, hdc);

	PatBlt(_hMemDC, 0, 0, Rect.right, Rect.bottom, WHITENESS);
}
void DrawMemDC(HWND hWnd) {

	////-------------------------------------
	//// 메모리DC 사용 부분
	////-------------------------------------
	// 그릴공간세팅
	PAINTSTRUCT  ps;
	RECT rect;
	HDC hdc;
	int dataX=0;
	GetClientRect(hWnd, &rect);                                 // 윈도우 크기 얻어오기
	PatBlt(_hMemDC, 0, 0, rect.right, rect.bottom, WHITENESS);  // 버퍼 다지우기 (하얀색 사각형으로 만듦)

	// 그림그리기 코드
	//MoveToEx(_hMemDC, 10, 10, NULL);
	//LineTo(_hMemDC, _iOldX, _iOldY);

	// 그림 그리기
	auto iter = _DataList.begin();
	MoveToEx(_hMemDC, dataX, *iter, NULL);
	while (iter != _DataList.end()) {
		LineTo(_hMemDC, dataX, *iter);
		dataX += X_AXISUNCREMENT;
		++iter;
	}
	// 갱신
	InvalidateRect(hWnd, NULL, FALSE);
	// WM_PAINT 메시지
	/*
	case WM_PAINT:
		// 플립
		GetClientRect(hWnd, &rect);
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, rect.right, rect.bottom, _hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	*/
}
void DestroyMemDC() {
	//-------------------------------------
	// 메모리DC 파괴 부분
	//-------------------------------------
	SelectObject(_hMemDC, _hMemBitmapOld);
	DeleteObject(_hMemBitmap);
	DeleteObject(_hMemDC);
}



//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT  ps;
	RECT rect;
	HDC hdc;
	switch (message) {
	case WM_CREATE:
		SetTimer(hWnd, 0, 5, (TIMERPROC) TimerProc);
		break;
	case WM_SIZE:
		DestroyMemDC();
		CreateMemDC(hWnd);
		break;
	case WM_PAINT:
		// 플립
		GetClientRect(hWnd, &rect);
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, rect.right, rect.bottom, _hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		DestroyMemDC();
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
