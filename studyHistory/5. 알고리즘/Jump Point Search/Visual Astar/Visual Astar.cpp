// Visual Astar.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Visual Astar.h"
#include "DrawMap.h"
#include <windowsx.h>
#include "CFramework.h"
// Global Variables:
HINSTANCE hInst;                                // current instance
HWND g_hWnd;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {

	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VISUALASTAR));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_VISUALASTAR);
	wcex.lpszClassName = L"Visual JumpPoint";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	// Perform application initialization:
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(L"Visual JumpPoint", L"Visual JumpPoint Title", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VISUALASTAR));

	MSG msg;

	// Main message loop:
	BOOL bDone = FALSE;
	while (!bDone) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				bDone = TRUE;
			}
		} else {
			CFramework::GetInstance()->Running();
			CFramework::GetInstance()->Display();
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
	int xPos;
	int yPos;
	switch (message) {
	case WM_CREATE:
		CFramework::GetInstance()->Init(hWnd);
		g_hWnd = hWnd;

		break;


	case WM_LBUTTONDBLCLK:  // ?????? ????????????
	{
		// ???????????? ??????
		xPos = GET_X_LPARAM(lParam);
		yPos = GET_Y_LPARAM(lParam);
		CFramework::GetInstance()->SetStartPos(xPos, yPos);
	}
	break;
	case WM_RBUTTONDBLCLK: // ????????? ????????????
	{
		// ????????? ??????
		xPos = GET_X_LPARAM(lParam);
		yPos = GET_Y_LPARAM(lParam);
		CFramework::GetInstance()->SetEndPos(xPos, yPos);
	}
	break;
	case WM_LBUTTONDOWN: // ???????????? ??????
	{
		// ?????????/?????? ??????
		xPos = GET_X_LPARAM(lParam);
		yPos = GET_Y_LPARAM(lParam);
#ifndef dfBRESENHAM


		CFramework::GetInstance()->LButtonDown(xPos, yPos);
		CFramework::GetInstance()->ToggleWall(xPos, yPos);
#else
		CFramework::GetInstance()->StartLine(xPos, yPos);
#endif // !dfBRESENHAM

	}
	break;
	case WM_LBUTTONUP: // ???????????? ???
		// ?????????/?????? ???
		CFramework::GetInstance()->LButtonUp();
		break;
	case WM_MOUSEMOVE:
	{
		// ??????????????? ????????? ??????????????? ?????????/??????
		xPos = GET_X_LPARAM(lParam);
		yPos = GET_Y_LPARAM(lParam);
#ifndef dfBRESENHAM

		CFramework::GetInstance()->ToggleWall(xPos, yPos);	// ?????????
#else
		CFramework::GetInstance()->DrawLine(xPos, yPos);	// ?????????
#endif
	}
	break;
	case WM_MBUTTONDOWN:
		// ????????? ??????
		// ????????? ??????
		CFramework::GetInstance()->FindPath();
		break;
	case WM_KEYDOWN:
		// ????????? ??????
		switch (LOWORD(wParam)) {
		case VK_SPACE:
			// ???????????????
			// ??? ?????? ??????
			CFramework::GetInstance()->ResetMap();
			break;
		case 'M':
			// m??? ????????? ?????????
			CFramework::GetInstance()->GenerateMap();
			break;
		case 'R':
			// ????????? ????????????
			CFramework::GetInstance()->LoadMap();
			break;
		case 'A':
			// ?????? ??????
			CFramework::GetInstance()->ToggleVerif();
			break;
		default:
			break;
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
