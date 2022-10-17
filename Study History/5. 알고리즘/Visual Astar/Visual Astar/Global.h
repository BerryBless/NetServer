#pragma once
#include <stdio.h>
#include "DrawMap.h"
#include "CMemory.h"
#include "CList.h"

#define dfRectLen 16					// 한변의 길이
#define dfMAPWIDTH 1400 / dfRectLen		// 화면 크기에 따른 너비 최대노드 개수
#define dfMAPHEIGHT 700 / dfRectLen 	// 화면 크기에 따른 높이 최대노드 개수

//#define dfPRINTINFO

enum class eMAP {
	Empty = 0,	// 빈공간
	Wall,		// 벽
	Start,		// 시작지점
	End,		// 끝지점
	Open,		// 예약 지점
	Close,		// 방문한 지점
	MAX,
};

struct stPOS {
	int X;
	int Y;
};

struct stPATH {
	stPATH *pParent;
	int X;
	int Y;
	int G;
	// H = F - G
	int F;
}  ; 

extern short g_map[dfMAPHEIGHT][dfMAPWIDTH];

