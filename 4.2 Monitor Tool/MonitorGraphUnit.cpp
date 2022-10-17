// MonitorGraphUnit.cpp : Defines the entry point for the application.
//
#include "pch.h"
#include "framework.h"
#include "MonitorGraphUnit.h"
#include "CMonitorGraphUnit.h"
#include "CMonitoringTool.h"
#define MAX_LOADSTRING 100

CMonitoringTool _tool(L"MonitoringToolConfig.ini");

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
	wcex.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Parent";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	// Perform application initialization:
	g_hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(L"Parent", L"Server Monitoring Tool",  
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
		nullptr, nullptr, hInstance, nullptr);

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


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		ASSERT_CRASH(_tool.ConnectMonitor());
		_tool.CreateView(g_hInst, hWnd);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_GETMINMAXINFO:
		((LPMINMAXINFO) lParam)->ptMinTrackSize.x = 1280;
		((LPMINMAXINFO) lParam)->ptMinTrackSize.y = 720;
		((LPMINMAXINFO) lParam)->ptMaxTrackSize.x = 1208;
		((LPMINMAXINFO) lParam)->ptMaxTrackSize.y = 720;

		((LPMINMAXINFO) lParam)->ptMaxSize.x = 1280;
		((LPMINMAXINFO) lParam)->ptMaxSize.y = 720;
		((LPMINMAXINFO) lParam)->ptMaxPosition.x = 1280;
		((LPMINMAXINFO) lParam)->ptMaxPosition.y = 720;

		break;
		
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}