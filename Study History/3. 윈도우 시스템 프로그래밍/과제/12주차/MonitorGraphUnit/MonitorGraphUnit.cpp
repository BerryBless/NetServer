// MonitorGraphUnit.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "MonitorGraphUnit.h"
#include "CMonitorGraphUnit.h"
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;                                // current instance

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MONITORGRAPHUNIT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MONITORGRAPHUNIT);
	wcex.lpszClassName = L"Parent";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	// Perform application initialization:
	g_hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(L"Parent", L"PWindow", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MONITORGRAPHUNIT));

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

CMonitorGraphUnit *p1;
CMonitorGraphUnit *p2;
CMonitorGraphUnit *p3;
CMonitorGraphUnit *p4;

int _cnt = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		p1 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::LINE_SINGLE, 10, 10, 200, 200);
		p2 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::LINE_SINGLE, 220, 10, 200, 200);
		p3 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::LINE_SINGLE, 430, 10, 400, 200);
		p4 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::LINE_SINGLE, 10, 220, 300, 250);
		SetTimer(hWnd, 1, 100, NULL);
	case WM_TIMER:
		p1->InsertData(_cnt);
		p2->InsertData(_cnt);
		p3->InsertData(_cnt);
		p4->InsertData(_cnt);
		++_cnt;
		_cnt %= 100;
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}