// MouseLine.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "MouseLine.h"
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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOUSELINE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MOUSELINE); // 리소스에 있는 메뉴
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
// 팬
HPEN _hPen;

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		// 내가 하고싶은 메시지가 오면 가로체서 우리가 처리
		// 외우진 말고 MSDN 보고 작성하기
	case WM_CREATE:
		// 윈도우 생성될때 (CreateWindowW() 내부에서 메시지 처리 발생)
		// 윈도우에 대한 초기화
		// 로직적인건 하지말기
		break;
	break;
	case WM_DESTROY:
		// 윈도우 파괴 종료메시지
		// 윈도우(API)와 프로세스는 다른것
		// 윈도우를 안만들어도 프로세스는 돎
		// 안꺼지면 세부정보 가서 프로그램 끄기
		// 
		// PostQuitMessage(0) : 종료메시지를 메시지큐에 넣음
		PostQuitMessage(0);
		break;

	case WM_LBUTTONDOWN:
		_mClick = true;
		break;
	case WM_LBUTTONUP:
		_mClick = false;
		break;
	case WM_RBUTTONDOWN:
		// 오른쪽 버튼다운이면 색상, 크기 랜덤으로
	{
		HDC hdc = GetDC(hWnd);

		// 펜생성 및 바꾸기
		DeleteObject(_hPen);// 기존팬 지우고
		_hPen = CreatePen(PS_SOLID, rand()%10, RGB(rand() % 255, rand() % 255, rand() % 255));	// 랜덤으로 팬생성
		ReleaseDC(hWnd, hdc);//팬바꿔주기
	}
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

		if (_mClick) {
			HDC hdc = GetDC(hWnd);

			(HPEN) SelectObject(hdc, _hPen);		// 하나만 선택가능 기존에 있던거 리턴

			// 직선 선그리기
			MoveToEx(hdc, _iOldX, _iOldY, NULL);	// 시작점
			LineTo(hdc, xPos, yPos);				// 끝점

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

