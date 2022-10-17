#pragma once
#include <Windows.h>
class CPaint {
private :
	HWND _hWnd;	// 윈도우 핸들

	HPEN _hLinePen;		// 노드를 이을 팬
	HPEN _hNodePen;		// 노드 테두리, 텍스트
	
	
	HBRUSH _hRedNodeBrush;	// 레드노드 채울 색
	HBRUSH _hBlackNodeBrush;// 블랙노드 채울 색

public:
	CPaint();
	~CPaint();

	void SetHWND(HWND hWnd);

	void PrintLine(int sX, int sY, int dX, int dY);
	void PrintNode(int iX, int iY, int iD, int iData, BYTE byColor);



};

