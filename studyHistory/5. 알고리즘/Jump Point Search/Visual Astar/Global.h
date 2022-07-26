#pragma once
#include <stdio.h>
#include "DrawMap.h"
#include "CMemory.h"
#include "CList.h"

#define dfRectLen 32			// 한변의 길이
#define dfMAPWIDTH 1600/dfRectLen		//  너비 최대노드 개수 1900 / dfRectLen
#define dfMAPHEIGHT 700/dfRectLen		//  높이 최대노드 개수 1000 / dfRectLen

//#define dfPRINTINFO // 노드의 F G H 정보 출력
//#define dfBRESENHAM // 브레즈함 알고리즘 테스트

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
	int F;					// 4
	int G;					//
	int Checksum; // (방향 검사 하위 8bit) 나머지는 패딩때문에 그냥..
	// H = F - G
	int X;					// 
	int Y;					// 4
	stPATH *pParent;		// 8

};

extern short g_map[dfMAPHEIGHT][dfMAPWIDTH];

