#include "CFramework.h"
#include <winuser.h>
#include "Profiler.h"

CFramework::CFramework() {
	// 플레그
	_bClick = false;
	_bWallCreate = false;
	_bLineCreate = false;

	// 시작점
	_startPos.X = 0;
	_startPos.Y = 0;
	g_map[_startPos.Y][_startPos.X] = (short)eMAP::Start;

	_endPos.X = 1;
	_endPos.Y = 0;
	g_map[_endPos.Y][_endPos.X] = (short)eMAP::End;
}
CFramework::~CFramework() {
}

void CFramework::Init(HWND hWnd) {
	_displayMap.Init(hWnd);
	_hWnd = hWnd;
}

void CFramework::ResetMap() {
	memset(g_map, 0, sizeof(g_map));
	g_map[_startPos.Y][_startPos.X] = (short)eMAP::Start;
	g_map[_endPos.Y][_endPos.X] = (short)eMAP::End;
	_path.clear();
	_astar.clear();
}

void CFramework::ClearMap() {
	for (int i = 0; i < dfMAPHEIGHT; i++) {
		for (int j = 0; j < dfMAPWIDTH; j++) {
			if (g_map[i][j] == (short)eMAP::Open ||
				g_map[i][j] == (short)eMAP::Close)
				// 우선순위큐 실행결과 지워주기
				g_map[i][j] = (short)eMAP::Empty;
		}
	}
	g_map[_startPos.Y][_startPos.X] = (short)eMAP::Start;
	g_map[_endPos.Y][_endPos.X] = (short)eMAP::End;
	_path.clear();
	_astar.clear();
}

void CFramework::ClearData() {
	g_map[_startPos.Y][_startPos.X] = (short)eMAP::Start;
	g_map[_endPos.Y][_endPos.X] = (short)eMAP::End;
	_path.clear();
	_astar.clear();
}

void CFramework::ToggleWall(int x, int y) {
	if (_bClick == false) return;
	// 좌표 구하기
	x /= dfRectLen;
	y /= dfRectLen;
	// 이미 선점중인건 건너뛰기
	if (g_map[y][x] == (short)eMAP::Start || g_map[y][x] == (short)eMAP::End) return;
	// 지금 바꿔야할것!
	if (_bWallCreate == false) {
		g_map[y][x] = (short)eMAP::Empty;
	} else {
		g_map[y][x] = (short)eMAP::Wall;
	}
}

void CFramework::Display() {
	// 그리드
	_displayMap.Display();
	// 경로
	if (!_path.empty())
		_displayMap.DrawPath(_path);
	// TODO 칸정보
	// 플립하기
	_displayMap.FlipMemDC();
}

void CFramework::SetStartPos(int x, int y) {
	// 시작지점 정하기
	x /= dfRectLen;
	y /= dfRectLen;
	g_map[_startPos.Y][_startPos.X] = (short)eMAP::Empty;
	_startPos.X = x;
	_startPos.Y = y;
	g_map[_startPos.Y][_startPos.X] = (short)eMAP::Start;
}

void CFramework::SetEndPos(int x, int y) {
	// 끝지점 정하기
	x /= dfRectLen;
	y /= dfRectLen;
	g_map[_endPos.Y][_endPos.X] = (short)eMAP::Empty;
	_endPos.X = x;
	_endPos.Y = y;
	g_map[_endPos.Y][_endPos.X] = (short)eMAP::End;
}

void CFramework::LButtonDown(int x, int y) {
	// 드레그 시작
	_bClick = true;

	x /= dfRectLen;
	y /= dfRectLen;
	if (g_map[y][x] == (short)eMAP::Wall) {
		// 시작지점에 벽있으면 지우기 모드로
		_bWallCreate = false;
	} else {
		// 비어있으면 벽세우기 모드로
		_bWallCreate = true;
	}

}

void CFramework::LButtonUp() {
	// 드레그 끝
	_bClick = false;
}

void CFramework::FindPath() {
	// 경로찾기
	// 빈칸, 시작, 끝, 벽만 남기기
	ClearMap();
	//_astar.Astar(_startPos, _endPos, _path); // 한번에 찾기
	_astar.Start(_startPos, _endPos);
	//PRO_PRINT("astar.log");
}

void CFramework::Running() {
	// 업데이트
	if (_astar.isWorking()) {
		_astar.Step();
		_astar.GetPath(_path);
	}
}


void CFramework::GenerateMap() {
	_mapGenerator.Generate();
}

void CFramework::DrawLine() {
	CList <stPOS> line;
		_dline.GetLine(_startPos.X, _startPos.Y, _endPos.X, _endPos.Y, line);
		for (auto iter = line.begin(); iter != line.end(); ++iter) {
			stPOS temp = *iter;
			g_map[temp.Y][temp.X] = (short) eMAP::MAX + 1;
		}
		line.clear();

}

struct stPATH *CFramework::GetPositionInfo(int x, int y) {
	return _astar.GetPathNode(x, y);
}

