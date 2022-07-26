#pragma once
#include "Global.h"
#include <Windows.h>

#include "CList.h"
class CDisplayMap {
private:

	HPEN _hRectPen;		// 사격형 팬(회색)
	HPEN _hLinePen;		// 경로 팬
	HPEN _hInfoPen;		// 칸 정보 팬
	
	HBRUSH _hBackBrush; // 배경색
	HBRUSH _hWallBrush;	// (short)eMAP::Wall (벽)
	HBRUSH _hStartBrush;	// (short)eMAP::Start (출발지)
	HBRUSH _hEndBrush;	// (short)eMAP::End (목적지)
	HBRUSH _hOpenBrush;	// (short)eMAP::Open (갈예정)
	HBRUSH _hCloseBrush;	// (short)eMAP::Close (갔던곳)

	// 윈도우정보
	int		_iWindowPosX;
	int		_iWindowPosY;
	int		_iWindowWidth;
	int		_iWindowHeight;

	int _half;	// 정사각형 한변의 길이


	//------------------------------------------------------
	// 내 윈도우 핸들
	//------------------------------------------------------
	HWND		_hWnd;

	//------------------------------------------------------
	// 더블 버퍼링용 메모리 DC, 메모리 비트맵
	//------------------------------------------------------
	HDC		_hMemDC;
	HBITMAP	_hMemBitmap;
	HBITMAP _hMemBitmapOld;

public:
	CDisplayMap();
	~CDisplayMap();
	void Init(HWND);
	void SetHWND(HWND);
	void Display();

	void DrawRect( int i, int j);		// 사각형 그리기
	void DrawPath(CList<struct stPOS> &path, HPEN pen);// 경로 그리기
	void Drawfrom(int pX, int pY, int vX, int vY, int len);
	void DrawLine(int sX, int sY, int eX, int eY);
	void DrawUI();

	// 더블버퍼링
	void CreateMemDC();
	void FlipMemDC();
	void DestroyMemDC();
	void Resize();
};

