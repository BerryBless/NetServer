// AsyncSelect_NetDraw_Server.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "AsyncSelect_NetDraw_Server.h"
#include <windowsx.h>
#include "NetworkCore.h"
#include "CMonitorGraphUnit.h"
#define dfSERVERFORT 25000
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {

	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASYNCSELECTNETDRAWSERVER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ASYNCSELECTNETDRAWSERVER);
	wcex.lpszClassName = L"CDrawServer";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	hInst = hInstance; // Store instance handle in our global variable

		// 윈도우 크기 조절 640x480
	RECT winSize = {0,0,640,480};
	AdjustWindowRect(&winSize, WS_OVERLAPPEDWINDOW, false); // 작업영역의 크기가 640x480 이다

	HWND hWnd = CreateWindowW(L"CDrawServer", L"DrawServer", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, winSize.right - winSize.left, winSize.bottom - winSize.top,
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ASYNCSELECTNETDRAWSERVER));



	MSG msg;

	// 서버 초기화
	NetworkInitServer();


	// Main message loop:
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			//  게임로직
			SelectProc();
		}
		Sleep(5);
	}

	NetworkCloseServer();
	return (int) msg.wParam;
}


CMonitorGraphUnit *g_pClientCount;
CMonitorGraphUnit *g_pPacketCount;



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
	switch (message) {
	case WM_CREATE:
		g_pClientCount = new CMonitorGraphUnit(hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, L"접속자", 10, 10, 200, 200,
			dfMAXPLAYER, 0);
		g_pPacketCount = new CMonitorGraphUnit(hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, L"패킷", 220, 10, 200, 200, 
			dfMAXPLAYER * 2, 0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

