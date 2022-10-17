#include "CMapGenerator.h"
#include <stdlib.h>
#include "CFramework.h"
CMapGenerator::CMapGenerator() {
}

CMapGenerator::~CMapGenerator() {
}

void CMapGenerator::Generate() {
	// 맵초기화
	// 모든 맵을 벽으로
	for (int i = 0; i < dfMAPHEIGHT; i++) {
		for (int j = 0; j < dfMAPWIDTH; j++) {
			g_map[i][j] = (short)eMAP::Wall;
		}
	}
	stPOS start;
	start.X = 0;
	start.Y = 0;

	RecursiveBacktracking(start);

	CFramework::GetInstance()->ClearData();

}

void CMapGenerator::RecursiveBacktracking(stPOS now) {
	int r = rand() % 4;
	int ni; // next Index
	stPOS next;		// 다음 갈위치


	g_map[now.Y][now.X] = (short)eMAP::Empty;

	for (int i = 0; i < 4; i++) {
		// 4방향중 랜덤 선택
		ni = (r + i) % 4;

		// 다음방향
		next.X = now.X + _dx[ni];
		next.Y = now.Y + _dy[ni];

		// 무결성 체크
		if (0 > next.X || next.X >= dfMAPWIDTH)continue;
		if (0 > next.Y || next.Y >= dfMAPHEIGHT)continue;
		if (g_map[next.Y][next.X] == (short)eMAP::Empty) continue;


		// 못감(뚫으면 연결되기 때문에
		if (RecursiveCanGo(next, ni))
			RecursiveBacktracking(next);
	}
}

bool CMapGenerator::RecursiveCanGo(stPOS now, int dir) {
	// 진행방향의 5면이 벽이어야함.
	//    벽  벽
	//    ->  벽   
	//    벽  벽

	//  벽 벽 벽 
	//  벽 ^  벽
	//     |

	stPOS check;

	// 진행방향
	check.X = now.X + _dx[dir];
	check.Y = now.Y + _dy[dir];
	if (-1 < check.X && check.X < dfMAPWIDTH &&
		-1 < check.Y && check.Y < dfMAPHEIGHT)
		if (g_map[check.Y][check.X] == (short)eMAP::Empty)
			return false;

	// 진행방향의 왼쪽
	check.X = now.X + _dx[(dir + 3) % 4];
	check.Y = now.Y + _dy[(dir + 3) % 4];
	if (-1 < check.X && check.X < dfMAPWIDTH &&
		-1 < check.Y && check.Y < dfMAPHEIGHT)
		if (g_map[check.Y][check.X] == (short)eMAP::Empty)
			return false;


	// 진행방향의 오른쪽
	check.X = now.X + _dx[(dir + 1) % 4];
	check.Y = now.Y + _dy[(dir + 1) % 4];
	if (-1 < check.X && check.X < dfMAPWIDTH &&
		-1 < check.Y && check.Y < dfMAPHEIGHT)
		if (g_map[check.Y][check.X] == (short)eMAP::Empty)
			return false;

	// 대각선체크
	/*일부로 공간 창출
	if (_dy[dir] == 0) {
		// 수평방향

		// 위 대각선
		check.X = now.X + _dx[dir];
		check.Y = now.Y + _dy[(dir + 3) % 4];

		if (-1 < check.X && check.X < dfMAPWIDTH &&
			-1 < check.Y && check.Y < dfMAPHEIGHT)
			if (g_map[check.Y][check.X] == (short)eMAP::Empty)
				return false;

		// 아래 대각선
		check.X = now.X + _dx[dir];
		check.Y = now.Y + _dy[(dir + 1) % 4];

		if (-1 < check.X && check.X < dfMAPWIDTH &&
			-1 < check.Y && check.Y < dfMAPHEIGHT)
			if (g_map[check.Y][check.X] == (short)eMAP::Empty)
				return false;
		//

	} else {
		// 수직방향

		// 왼 대각선
		check.X = now.X + _dx[(dir + 3) % 4];
		check.Y = now.Y + _dy[dir];

		if (-1 < check.X && check.X < dfMAPWIDTH &&
			-1 < check.Y && check.Y < dfMAPHEIGHT)
			if (g_map[check.Y][check.X] == (short)eMAP::Empty)
				return false;

		// 오른 대각선
		check.X = now.X + _dx[(dir + 1) % 4];
		check.Y = now.Y + _dy[dir];

		if (-1 < check.X && check.X < dfMAPWIDTH &&
			-1 < check.Y && check.Y < dfMAPHEIGHT)
			if (g_map[check.Y][check.X] == (short)eMAP::Empty)
				return false;
		//
	}*/
	return true;
}

