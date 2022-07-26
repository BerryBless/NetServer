// AsyncSelect_NetDraw_Client.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "AsyncSelect_NetDraw_Client.h"
#include "NetworkCore.h"
#include <windowsx.h>
#include "Resource.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR g_IPAddr[18] = L"127.0.0.1";


LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);// 다이얼로그

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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASYNCSELECTNETDRAWCLIENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ASYNCSELECTNETDRAWCLIENT);
	wcex.lpszClassName = L"CDrawClinet";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	// 윈도우 크기 조절 640x480
	RECT winSize = {0,0,640,480};
	AdjustWindowRect(&winSize, WS_OVERLAPPEDWINDOW, false); // 작업영역의 크기가 640x480 이다

	RegisterClassExW(&wcex);

	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(L"CDrawClinet", L"DrawClinet", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, winSize.right - winSize.left, winSize.bottom - winSize.top, // 윈도우 크기
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ASYNCSELECTNETDRAWCLIENT));

	MSG msg;

	

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	NetworkCloseClient();
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

BOOL g_mClick;
int g_iOldX;
int g_iOldY;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		if(DialogBox(hInst, MAKEINTRESOURCE(IDD_IPINPUT),
			hWnd, DialogProc) == IDOK){ 
			// 연결시도
			if (NetworkInitClient(hWnd, g_IPAddr) == FALSE) {
				NetworkCloseClient();
			} else {
			}
		}
		break;
	case UM_NETWORK:
		// TODO 에러처리
		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_READ:
			// rece버퍼에 무언가 남아있으면 FD_READ 메시지
			ReadEvent(hWnd);
			break;
		case FD_WRITE:
			// send() 불가능 했다거, 다시 가능해지는 타이밍
			// 1. 연결이 막 되었을때
			// 2. send()를 못보냈다가 보낼 수 있다고 판단했을때
			WriteEvent();
			break;
		case FD_CONNECT:
			// TODO connect 했을때
			break;
		case FD_CLOSE:
			// TODO close 했을때
			break;
		default:
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		g_mClick = TRUE;
		break;
	case WM_LBUTTONUP:
		g_mClick = FALSE;
		break;

	case WM_MOUSEMOVE:
		// 마우스가 움직일때마다
		// wParam : 키입력
		// lParam : 마우스 좌표
	{
		// 비트연산자로 좌표 쪼개기
		// #include <windowsx.h> 필요
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		char chpPacket[18];
		stHEADER *stpHeader =(stHEADER*) chpPacket;
		st_DRAW_PACKET *stpDrawPacket  = (st_DRAW_PACKET*) (chpPacket + sizeof(stHEADER) );
		if (g_mClick) {
			stpHeader->Len = sizeof(st_DRAW_PACKET);
			stpDrawPacket->iEndX = xPos;
			stpDrawPacket->iEndY = yPos;
			stpDrawPacket->iStartX = g_iOldX;
			stpDrawPacket->iStartY = g_iOldY;
			SendPacket(chpPacket, sizeof(chpPacket));
		}
		g_iOldX = xPos;
		g_iOldY = yPos;

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

INT_PTR DialogProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_IPINPUT, g_IPAddr);
		return (INT_PTR) TRUE;
	case WM_COMMAND:
		switch (wParam) {
		case IDOK: {
			GetDlgItemText(hDlg, IDC_IPINPUT, g_IPAddr, 18);
			EndDialog(hDlg, 1);
		}return (INT_PTR) TRUE;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return (INT_PTR) TRUE;
		}
		break;
	}
	return (INT_PTR) FALSE;
}

