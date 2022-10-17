// WindowsProject1.cpp : 애플리케이션에 대한 진입점을 정의합니다.
// http://soen.kr/
// 기본적으로 알아야 할것
// WM_DESTROY
// WM_SYSCOMAND
// WM_PAINT
// WM_MOUSEMOVE
// 
// WM_LBUTTONDOWN / UP
// MessageBox
// 
// MoveToEx / LineTo
// CreatePen
// CreateBursh
// CreateFont
// Timer WM_TIMER
// 
// InvalidateRect
// UpdateWindow
// 
// SetPixel
//

#include "framework.h"
#include "WindowsProject1.h"
#include <windowsx.h>


#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
//스트링테이블 안씀!
//WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
//WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);	// 타이머용
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM); // 다이얼로그용

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	// hPrevInstance, lpCmdLine안씀
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	// nCmdShow : 최소화에서 실행, 전채화면으로 실행....
	_In_ int       nCmdShow) {


	// 클래스등록을 위한 구조체
	//MyRegisterClass(hInstance);
	// WNDCLASS "EX" W 윈도우 필수 요소 클레스 등록
	WNDCLASSEXW wcex;


	
	wcex.cbSize = sizeof(WNDCLASSEX); // 자기자신의 사이즈 (확장성을 위해 버전관리 대신)
	wcex.style = CS_HREDRAW | CS_VREDRAW;  // 비트로 설정
	wcex.lpfnWndProc = WndProc;  // 메시지처리 함수 포인터 등록
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1); // 리소스에 있는 메뉴
	wcex.lpszClassName = L"ClassName123";    // 매핑할 클래스 이름 지정 재사용할때 클래스이름 으로 불러옴
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	// 클래스 등록 시킵니다.
	RegisterClassExW(&wcex);

	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	// CreateWindowW / CreateWindow EX W 둘중 하나 사용하는 매크로 함수
	// WS_OVERLAPPEDWINDOW : 일반적인 외형 윈도우 스타일 (디파인 보기)
	// CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 좌표와 크기
	HWND hWnd = CreateWindowW(L"ClassName123", L"윈도우 타이틀", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		// GetLastError() : 가장 마지막의 에러코드 MSDN에 검색
		// 에러가 났으면 바로 호출! (printf도 하면안되고 디버깅으로 보자)
		// https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
		DWORD dwError = GetLastError();
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	/*
typedef struct tagMSG {
	HWND        hwnd;
	UINT        message;
	WPARAM      wParam; // 파라메타 그냥 크기의 변수를 받음
	LPARAM      lParam; // 파라메타 그냥 크기의 변수를 받음
	DWORD       time;
	POINT       pt;
#ifdef _MAC
	DWORD       lPrivate;
#endif
} MSG, *PMSG, NEAR *NPMSG, FAR *LPMSG;
	*/
	MSG msg;

	// 기본 메시지 루프입니다:
	// GetMessage(&msg, nullptr, 0, 0) : dequeue, 메시지 없으면 bolck상태, 종료 return 윈도우 종료 메시지가 오면 0을 리턴
	while (GetMessage(&msg, nullptr, 0, 0)) {
		/* 단축키 메시지 처리
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
		}*/
		TranslateMessage(&msg); // 메시지 나누기(입력_ 키다운, 겟차)/ 변화시키기
		DispatchMessage(&msg);	// 메시지의 WndProc 호출
	}

	return (int) msg.wParam;
}

// 마우스 과거값
int _iOldX;
int _iOldY;
// 마우스 클릭 플레그
bool _mClick;


//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//

void CALLBACK   TimerProc(HWND hWnd, UINT message, UINT_PTR wParam, DWORD lParam) {
	switch (wParam) {
	case 0:
	{
		HDC hDC = GetDC(hWnd);
		SetPixel(hDC, rand() % 100, rand() % 100, RGB(255, 0, 0));
		ReleaseDC(hWnd, hDC);
	}
	break;
	case 1:
	{
		HDC hDC = GetDC(hWnd);
		SetPixel(hDC, rand() % 100, rand() % 100, RGB(0, 255, 0));
		ReleaseDC(hWnd, hDC);
	}
	break;
	case 2:
	{
		HDC hDC = GetDC(hWnd);
		SetPixel(hDC, rand() % 100, rand() % 100, RGB(0, 0, 255));
		ReleaseDC(hWnd, hDC);
	}
	break;
	default:
		break;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		// 내가 하고싶은 메시지가 오면 가로체서 우리가 처리
		// 외우진 말고 MSDN 보고 작성하기
	case WM_CREATE:
		// 윈도우 생성될때 (CreateWindowW() 내부에서 메시지 처리 발생)
		// 윈도우에 대한 초기화
		// 로직적인건 하지말기
		SetTimer(hWnd, 0, 1, (TIMERPROC) TimerProc);
		break;
	//case WM_TIMER:
		//switch (wParam) {
		//case 0:
		//{
		//	HDC hDC = GetDC(hWnd);
		//	SetPixel(hDC,rand()% 100,rand()%100,RGB(255,0,0));
		//	ReleaseDC(hWnd, hDC);
		//}
		//break;
		//case 1:
		//{
		//	HDC hDC = GetDC(hWnd);
		//	SetPixel(hDC, rand() % 100, rand() % 100, RGB(0, 255, 0));
		//	ReleaseDC(hWnd, hDC);
		//}
		//break;
		//case 2:
		//{
		//	HDC hDC = GetDC(hWnd);
		//	SetPixel(hDC, rand() % 100, rand() % 100, RGB(0, 0, 255));
		//	ReleaseDC(hWnd, hDC);
		//}
		//break;
		//default:
		//	break;
		//}

		//break;
	case WM_PAINT:
		// 뺄꺼면 그냥 WM_PAINT 다빼
		// 윈도우를 갱신하라 (Render)
		// 1. 최초 한번
		// 2. 상호동작
		// *3. 다른윈도우가 가릴때 - win10 은 반투명효과 에선 발생안됨* 
		// 4. 최소 최대화
		// *5. 내가 직접 WM_PAINT 메시지를 발생시킬때*
	{


		//typedef struct tagPAINTSTRUCT {
		//	HDC         hdc;// 출력대상 장치의 핸들
		//	BOOL        fErase;
		//	RECT        rcPaint;	// 일부의 갱신될곳(가려졌다 나타낸것) 사각형 (x,y,x,y) 쓸려면 써~
		//	BOOL        fRestore;
		//	BOOL        fIncUpdate;
		//	BYTE        rgbReserved[32];
		//} PAINTSTRUCT, *PPAINTSTRUCT, *NPPAINTSTRUCT, *LPPAINTSTRUCT;
		PAINTSTRUCT ps;
		// 윈도우한테 그리기 시작한다 알림. 없으면 계속 일할때까지 발생
		HDC hdc = BeginPaint(hWnd, &ps);

		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
		TextOut(hdc, 10, 10, L"1234", 3);

		// 윈도우한테 그리기 끝났다 알림.
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		// 윈도우 파괴 종료메시지
		// 윈도우(API)와 프로세스는 다른것
		// 윈도우를 안만들어도 프로세스는 돎
		// 안꺼지면 세부정보 가서 프로그램 끄기
		// 
		// PostQuitMessage(0) : 종료메시지를 메시지큐에 넣음
		PostQuitMessage(0);
		
		// 타이머 죽이기
		KillTimer(hWnd, 0);
		KillTimer(hWnd, 1);
		KillTimer(hWnd, 2);
		break;
	case WM_SYSCOMMAND:
		// 예제) 메시지 가로체고 종료메시지박스 띄우기

		if (wParam == SC_CLOSE) {
			if (IDOK ==
				MessageBox(NULL, L"종료 하시겠습니까?", L"종료", MB_OKCANCEL)) {
				return DefWindowProc(hWnd, message, wParam, lParam);
			}

			return 0;
		}
		{
			HDC hdc = GetDC(hWnd);
			TextOut(hdc, 100, 100, L"asd", 3);
			ReleaseDC(hWnd, hdc);
		}
		DefWindowProc(hWnd, message, wParam, lParam);
		break;

	case WM_LBUTTONDOWN:
		_mClick = true;
		break;
	case WM_LBUTTONUP:
		_mClick = false;
		break;
	case WM_MOUSEMOVE:
		// 마우스가 움직일때마다
		// wParam : 키입력
		// lParam : 마우스 좌표

	{
		// 비트연산자로 쪼개기
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if(_mClick) {
			HDC hdc = GetDC(hWnd);
			// SetPixel(hdc, xPos, yPos, RGB(222, 111, 0));
			
			// 펜생성 및 바꾸기
			HPEN hPen= CreatePen(PS_SOLID, 10, RGB(0, 0, 255));	// 팬 리소스 핸들
			HPEN hPenOld = (HPEN)SelectObject(hdc, hPen);		// 하나만 선택가능 기존에 있던거 리턴

			// 직선 선그리기
			MoveToEx(hdc, _iOldX, _iOldY, NULL);	// 시작점
			LineTo(hdc, xPos, yPos);			// 끝점
			
			// 펜지우기
			SelectObject(hdc, hPenOld);			// 기존으로 연결 끊기
			DeleteObject(hPen);					// 펜지우기 (윈 7까지 연결 안끊으면 삭제안됐음)

			// DC연결 해지
			ReleaseDC(hWnd, hdc);

		}
		_iOldX = xPos;
		_iOldY = yPos;

	}
	break;

	default:
		// 내가 처리 하지 않은 기본적인 메시지 처리기
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

