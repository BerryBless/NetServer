// memdc.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "memdc.h"
#include <windowsx.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);	// 타이머용
void CreateMemDC(HWND hWnd);
void DestroyMemDC();
//-------------------------------------
// 전역에 메모리DC 핸들 보관
//-------------------------------------
HDC	_hMemDC;
HBITMAP	_hMemBitmap;
HBITMAP _hMemBitmapOld;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

    // TODO: Place code here.
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MEMDC));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MEMDC);
    wcex.lpszClassName = L"DoubleDC";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);


    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(L"DoubleDC", L"DoubleBuffering", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        DWORD dwError = GetLastError();
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 버퍼링 생성
    CreateMemDC(hWnd);


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MEMDC));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

int _iOldX;
int _iOldY;


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
    GetClientRect(hWnd, &rect);                                 // 윈도우 크기 얻어오기
    PatBlt(_hMemDC, 0, 0, rect.right, rect.bottom, WHITENESS);  // 버퍼 다지우기 (하얀색 사각형으로 만듦)

    // 그림그리기 코드
    MoveToEx(_hMemDC, 10,10, NULL);
    LineTo(_hMemDC, _iOldX, _iOldY);


    // 갱신
    InvalidateRect(hWnd, NULL, FALSE);

    // 플립
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





void CALLBACK   TimerProc(HWND hWnd, UINT message, UINT_PTR wParam, DWORD lParam) {
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rect;
    PAINTSTRUCT  ps;
    HDC hdc;
    switch (message)
    {
    case WM_CREATE:
        //SetTimer(hWnd, 0,15, (TIMERPROC) TimerProc);
        break;

    case WM_MOUSEMOVE: 
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        DrawMemDC(hWnd);

        

        _iOldX = xPos;
        _iOldY = yPos;

        InvalidateRect(hWnd, NULL, FALSE);
    }
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