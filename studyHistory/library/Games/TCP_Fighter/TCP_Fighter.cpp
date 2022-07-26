// TCP_Fighter.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "TCP_Fighter.h"
#include "Managers.h"
#include "CBaseObject.h"
#include "CPlayerObject.h"
#include "CEffectObject.h"
#include <windowsx.h>
#include "resource.h"
#include "NetworkCore.h"

#define MAX_LOADSTRING 100

// Global Variables:
extern BOOL g_bActiveApp;							// 이 윈도우가 활성화 되있냐 (원본 : CFramework.cpp)
HINSTANCE g_hInst;                                // current instance
HWND g_hWnd;


#ifndef dfSERVERIP
// 서버 IP선택
WCHAR g_IPAddr[18] = L"127.0.0.1";
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);// 다이얼로그
#endif // !dfSERVERIP

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL NetworkProc(WPARAM, LPARAM);

//TEMP
//#define TEMP
#ifdef TEMP

#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {


	// Initialize global strings
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TCPFIGHTER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"WinMain";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	// 윈도우 크기 조절 640x480
	RECT winSize = {0,0,640,480};
	AdjustWindowRect(&winSize, WS_OVERLAPPEDWINDOW, false); // 작업영역의 크기가 640x480 이다

	// Perform application initialization:
	g_hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(L"WinMain", L"WinMainTitle", WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, winSize.right - winSize.left, winSize.bottom - winSize.top,
		NULL, NULL, hInstance, NULL);

	if (!hWnd) {
		return FALSE;
	}
	g_hWnd = hWnd;


	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);


	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TCPFIGHTER));

	MSG msg;

	// 프레임워크 인스턴스
	CFramework *frameworkInst = I_FRAMEWORK;
	//INSTANTIATE(new CPlayerObject(0, 0, 50, 50, 100));

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
			//  게임로직
			// 키입력
			frameworkInst->KeyProcess();
			// 메인로직
			frameworkInst->Update();


			// 렌더링 스킵할지 결정 (시간재기)
			if (frameworkInst->FrameSkip()) {
				// 랜더링 가능!
				frameworkInst->Render();
				// FLIP
				I_SCREENDIB->DrawBuffer(g_hWnd, 0, 0);
			}
			// 타이틀에 로직FPS표시
			WCHAR wcpLFrame[5];
			wsprintf(wcpLFrame, L"%d", frameworkInst->GetLogicFPS());
			//TextOut(hDC, 0, 0, wcpLFrame, (int) wcslen(wcpRFrame));
			SetWindowText(g_hWnd, wcpLFrame);
		}
	}

	return (int) msg.wParam;
}

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
	{
#ifndef dfSERVERIP
		// 윈도우 생성 될때!
		// 다이얼로그로 연결할 곳 IP물어보기
		if (DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CONNECT_IP),
			hWnd, DialogProc) == IDOK) {
			// 연결시도
			if (NetworkInitClient(hWnd, g_IPAddr) == FALSE) {
				// 실패!
				MessageBox(hWnd, L"연결실패!", L"연결실패함!", MB_OK);
				NetworkCloseClient();
				DestroyWindow(hWnd);
			}
		} else {
			// 다이얼로그 취소버튼
			DestroyWindow(hWnd);
		}
#else
		// 서버 아이피 선언 되어있음
		if (NetworkInitClient(hWnd, dfSERVERIP) == FALSE) {
			// 실패!
			NetworkCloseClient();
			DestroyWindow(hWnd);
		}
#endif // !dfSERVERIP
	}
	break;
	case WM_ACTIVATEAPP:
		// 현재 창이 활성화 되어있는지
		g_bActiveApp = (BOOL) wParam;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case UM_NETWORK:
	{
		// 네트위크 유저메시지
		if (NetworkProc(wParam, lParam) == FALSE) 			{
			MessageBox(hWnd, L"끊김!", L"암튼끊김!", MB_OK);
			//  모든걸 정리하는 코드
			NetworkCloseClient();
			DestroyWindow(hWnd);
		}
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

BOOL NetworkProc(WPARAM wParam, LPARAM lParam) {
	// 에러처리
	if (WSAGETSELECTERROR(lParam) != 0) {
		// 에러 발생! 종료!
		return FALSE;
	}
	// lParam에 AsyncSelect메시지 들어있음!
	switch (WSAGETSELECTEVENT(lParam)) {
	case FD_READ:
		// rece버퍼에 무언가 남아있으면 FD_READ 메시지
		ReadEvent();
		return TRUE;
	case FD_WRITE:
		// send() 불가능 했다거, 다시 가능해지는 타이밍
		// 1. 연결이 막 되었을때
		// 2. send()를 못보냈다가 보낼 수 있다고 판단했을때
		WriteEvent();
		return TRUE;
	case FD_CONNECT:
		// TODO connect 했을때
		// 연결 성공처리
		return TRUE;
	case FD_CLOSE:
		// close 했을때
		// 그냥 종료
		return FALSE;
		//break;
	}
	return TRUE;
}


#ifndef dfSERVERIP
// 다이얼로그!
INT_PTR DialogProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_INITDIALOG:
		// 생성할때
		SetDlgItemText(hDlg, IDC_IP_TEXTBOX, g_IPAddr);
		return (INT_PTR) TRUE;
	case WM_COMMAND:
		switch (wParam) {
		case IDOK: {
			// OK버튼
			GetDlgItemText(hDlg, IDC_IP_TEXTBOX, g_IPAddr, 18);
			EndDialog(hDlg, 1);
		}return (INT_PTR) TRUE;
		case IDCANCEL:
			// CANCEL버튼
			EndDialog(hDlg, 0);
			return (INT_PTR) TRUE;
		}
		break;
	}
	return (INT_PTR) FALSE;
}

#endif // !dfSERVERIP