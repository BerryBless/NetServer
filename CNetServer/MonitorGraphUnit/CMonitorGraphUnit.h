/*
모니터링 프로그램의 자식윈도우 클래스

- 본 클래스 내부에서 자식 윈도우의 생성, 자식 윈도우의 프로시저, 데이터 등을 모두 가진다.
*/
#pragma once
#include "framework.h"
#include <list>
#define dfMAXCHILD		100	// 최대 자식수

#define dfFONT_TITLE_SIZE 20	// 타이틀 폰트 사이즈
#define dfFONT_TITLE_WIDTH 100	// 타이틀 폰트 너비
#define dfFONT_POINT_SIZE 15	// 그리드 숫자 폰트 사이즈
#define dfTITLE_RECT_HEIGHT 25 // 타이틀바 높이
#define dfALERT_TIME_MS	100		// 경고 타이머 시간

#define dfDATA_CPU_USED 10;
#define dfDATA_CPU_NETWORK_SEND 21;
#define dfDATA_CPU_NETWORK_RECV 22;


class CMonitorGraphUnit {
public:

	enum class TYPE {
		LINE_SINGLE,
		LINE_MULTI,
		NUMBER,
		ONOFF,
		PIE
	};


public:
	// 프레임p

	// 생성 소멸
	CMonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, const WCHAR *szTitle, int iPosX, int iPosY, int iWidth, int iHeight, int iDataMax, int iDataAlert);
	~CMonitorGraphUnit();

	/////////////////////////////////////////////////////////
	// 윈도우 프로시저
	/////////////////////////////////////////////////////////
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/////////////////////////////////////////////////////////
	// 데이터 넣기.
	/////////////////////////////////////////////////////////
	BOOL	InsertData(int iData);

	// 화면 갱신
	void CreateMemDC();	// 그림 그릴 버퍼 생성

	// 버퍼에 그림 그리는 코드
	virtual void Paint_LineSingle();	// 꺾은선 그래프
	//***
	// 여기에 원하는 그래프 추가 생성코드 그때그때 짜기
	//***
	virtual void Paint_Title(); // 타이틀바
	virtual void Paint_Grid();	// 격자

	void FlipMemDC();	// 버퍼그림을 모니터에 출력
	void DestroyMemDC();// 버퍼삭제

	// 번외
	int GetDataYPos(int data); // 높이로 y포지션 밑에서 부터 구하기
	void ChangeBackgroundBrush();

protected:

	//------------------------------------------------------
	// 윈도우 핸들, this 포인터 매칭 테이블 관리.
	//------------------------------------------------------
	BOOL				PutThis(void);
	static CMonitorGraphUnit *GetThis(HWND hWnd);


private:

	//------------------------------------------------------
	// 부모 윈도우 핸들, 내 윈도우 핸들, 인스턴스 핸들
	//------------------------------------------------------
	HWND		_hWndParent;
	HWND		_hWnd;
	HINSTANCE	_hInstance;

	//------------------------------------------------------
	// 윈도우 위치,크기,색상, 그래프 타입 등.. 자료
	//------------------------------------------------------
	// 윈도우 타입
	TYPE		_enGraphType;

	// 색상
	COLORREF	_BackColor;
	COLORREF _TitleColor;
	COLORREF _UIColor;

	// 윈도우정보
	int		_iWindowPosX;
	int		_iWindowPosY;
	int		_iWindowWidth;
	int		_iWindowHeight;

	//------------------------------------------------------
	// 더블 버퍼링용 메모리 DC, 메모리 비트맵
	//------------------------------------------------------
	HDC		_hMemDC;
	HBITMAP	_hMemBitmap;
	HBITMAP _hMemBitmapOld;

	//------------------------------------------------------
	// 리소스
	//------------------------------------------------------
	// 폰트
	HFONT _hFontTitle;
	HFONT _hFontGrid;

	// 브러쉬
	HBRUSH _hBackBrush;
	HBRUSH _hBackBrush_Title;

	// 펜
	HPEN _hLinePen;
	HPEN _hGridPen;

	//------------------------------------------------------
	// 데이터
	//------------------------------------------------------
	//데이터 큐 / 리스트
	std::list<int> _dataList;

	static int _childCnt;// 자식이 몇개있는지 카운터

	int _iDataMax; // 데이터 최대 크기
	int		_iDataAlert;	// 경보 울리는 기준값

	WCHAR _szTitle[30];// 타이틀 이름

		//------------------------------------------------------
	// 경고모드 - 
	//
	// 데이터 경고수치 도달시 경고음과 함께 자신을 제외한 모든 윈도우에 UM_ALERT 메시지를 보낸다.
	//
	// _bAlertMode = TRUE 가 된 후 타이머를 사용하여 일정시간 후 해제.
	//
	//  경고 상황을 굳이  맴버 변수 플래그로 들고 있는 이유는 ... ?  
	//  필요치 않다면 안만드셔도 되며,  구현하다 보면 필요하게 될 것입니다.  
	//------------------------------------------------------
	BOOL		_bAlertMode;
};

/*
///////////////////////////////////////////////////////////


* 위 클래스의 외부 사용법

CMonitorGraphUnit *p1;
CMonitorGraphUnit *p2;
CMonitorGraphUnit *p3;
CMonitorGraphUnit *p4;

- 임시로 포인터 변수 선언. 전역


case WM_CREATE:
  p1 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, 10, 10, 200, 200);
  p2 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, 220, 10, 200, 200);
  p3 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, 430, 10, 400, 200);
  p4 = new CMonitorGraphUnit(g_hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, 10, 220, 300, 250);
  SetTimer(hWnd, 1, 100, NULL);

- 부모 왼도우 WM_CREATE 에서 자식 윈도우 클래스들 생성.
- 타이머도 생성.


case WM_TIMER:
	p1->InsertData(rand() % 100);
	p2->InsertData(rand() % 100);
	p3->InsertData(rand() % 100);
	p4->InsertData(rand() % 100);
	break;

- 타이머는 랜덤하게 데이터를 생성하여 모든 윈도우로 전송.




* 자식 윈도우는 데이터를 받은 뒤 자신의 InvalidateRect 를 호출하고
* 자식 윈도우 클래스 내부의 윈도우 프로시저 WM_PAINT 에서 그래프를 그린다.



* 자식 윈도우 생성하는 코드

// 부모 윈도우
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

// 자식윈도우
_hWnd = CreateWindow(_szWindowClass, NULL, WS_CHILD | WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS,
						_iWindowPosX, _iWindowPosY, _iWindowWidth, _iWindowHeight, _hWndParent, NULL, _hInstance, NULL);


*/