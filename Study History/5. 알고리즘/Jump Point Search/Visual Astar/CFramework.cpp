#include "CFramework.h"
#include <winuser.h>
#include "Profiler.h"
#include <cstring>
#include <ctime>

CFramework::CFramework() {
	// 플레그
#ifdef dfBRESENHAM
	_bLine = false;			// 클릭
#endif
	_bClick = false;
	_bWallCreate = false;
	_bVerif = false;
	// 시작점
	_startPos.X = 0;
	_startPos.Y = 0;
	g_map[_startPos.Y][_startPos.X] = (short) eMAP::Start;

	_endPos.X = 1;
	_endPos.Y = 0;
	g_map[_endPos.Y][_endPos.X] = (short) eMAP::End;


	// 펜
	_pathPen = (HPEN) CreatePen(PS_SOLID, 6, RGB(0, 255, 255));
	_shortPathPen = (HPEN) CreatePen(PS_SOLID, 6, RGB(255, 0, 255));

	// 난수설정
	srand(80);
}
CFramework::~CFramework() {
	ResetMap();
}

void CFramework::Init(HWND hWnd) {
	_displayMap.Init(hWnd);
	_hWnd = hWnd;
}

void CFramework::ResetMap() {
	memset(g_map, 0, sizeof(g_map));
	g_map[_startPos.Y][_startPos.X] = (short) eMAP::Start;
	g_map[_endPos.Y][_endPos.X] = (short) eMAP::End;
	_path.clear();
	_corPath.clear();
	_astar.clear();
	_jps.clear();
}

void CFramework::ClearMap() {
	for (int i = 0; i < dfMAPHEIGHT; i++) {
		for (int j = 0; j < dfMAPWIDTH; j++) {
			if (g_map[i][j] == (short) eMAP::Open ||
				g_map[i][j] == (short) eMAP::Close ||
				g_map[i][j] >= (short) eMAP::MAX)
				// 우선순위큐 실행결과 지워주기
				g_map[i][j] = (short) eMAP::Empty;
		}
	}
	g_map[_startPos.Y][_startPos.X] = (short) eMAP::Start;
	g_map[_endPos.Y][_endPos.X] = (short) eMAP::End;
	_path.clear();
	_corPath.clear();
	_astar.clear();
	_jps.clear();
}

void CFramework::ClearData() {
	g_map[_startPos.Y][_startPos.X] = (short) eMAP::Start;
	g_map[_endPos.Y][_endPos.X] = (short) eMAP::End;
	_path.clear();
	_corPath.clear();
	_astar.clear();
	_jps.clear();
}

void CFramework::ToggleWall(int x, int y) {
	if (_bClick == false) return;
	// 좌표 구하기
	x /= dfRectLen;
	y /= dfRectLen;
	// 이미 선점중인건 건너뛰기
	if (g_map[y][x] == (short) eMAP::Start || g_map[y][x] == (short) eMAP::End) return;
	// 지금 바꿔야할것!
	if (_bWallCreate == false) {
		g_map[y][x] = (short) eMAP::Empty;
	} else {
		g_map[y][x] = (short) eMAP::Wall;
	}
}

void CFramework::Display() {
	// 그리드
	_displayMap.Display();
	// 경로
	if (!_path.empty()) {
		_displayMap.DrawPath(_path, _pathPen);
	}
	if (!_corPath.empty()) {
		_displayMap.DrawPath(_corPath, _shortPathPen);
	}

	// TEMP 브렌스헴?
#ifdef dfBRESENHAM
	if (_bLine) {
		_displayMap.DrawLine(_sX, _sY, _eX, _eY);
	}
#endif
	_displayMap.DrawUI();
	// 플립하기
	_displayMap.FlipMemDC();
}

void CFramework::SetStartPos(int x, int y, bool bWorld) {
	// 시작지점 정하기
	if (bWorld) {
		// 월드 좌표 -> 타일좌표
		x /= dfRectLen;
		y /= dfRectLen;
	}
	if (g_map[y][x] == (short) eMAP::End) return; // 끝지점이면 안바꿈
	g_map[_startPos.Y][_startPos.X] = (short) eMAP::Empty;
	_startPos.X = x;
	_startPos.Y = y;
	g_map[_startPos.Y][_startPos.X] = (short) eMAP::Start;
}

void CFramework::SetEndPos(int x, int y, bool bWorld) {
	// 끝지점 정하기
	if (bWorld) {
		// 월드 좌표 -> 타일좌표
		x /= dfRectLen;
		y /= dfRectLen;
	}
	if (g_map[y][x] == (short) eMAP::Start) return; // 시작지점이면 안바꿈
	g_map[_endPos.Y][_endPos.X] = (short) eMAP::Empty;
	_endPos.X = x;
	_endPos.Y = y;
	g_map[_endPos.Y][_endPos.X] = (short) eMAP::End;
}

void CFramework::LButtonDown(int x, int y) {
	// 드레그 시작
	_bClick = true;

	x /= dfRectLen;
	y /= dfRectLen;
	if (g_map[y][x] == (short) eMAP::Wall) {
		// 시작지점에 벽있으면 지우기 모드로
		_bWallCreate = false;
	} else {
		// 비어있으면 벽세우기 모드로
		_bWallCreate = true;
	}

}

void CFramework::LButtonUp() {
	// 드레그 끝
#ifndef dfBRESENHAM
	_bClick = false;
#else
	_bLine = false;
#endif // DEBUG

}

void CFramework::FindPath() {
	// 경로찾기
	// 빈칸, 시작, 끝, 벽만 남기기
	ClearMap();
	_jps.Start(_startPos, _endPos);
}

void CFramework::Running() {
	// 업데이트
	if (_bVerif == false) {
		if (_jps.isWorking()) {
			_jps.Step();
			if (_jps.isWorking() == false) {
				if (_jps.GetPath(_path)) {
					// 경로찾기 성공
					_correction.Correction(_path, _corPath); // 경로보정
				}
			}
			Sleep(500);
		}
	} else {
		// 자동검증!
		_iVcnt++;
		printf_s("=======================================\n");
		printf_s("LOOP : %d\n", _iVcnt);
		printf_s("Generatemap ..\n");
		// 맵생성
		_mapGenerator.Generate();

		printf_s("Set IN, OUT Position ..\n");
		SetStartPos(rand() % dfMAPWIDTH, rand() % dfMAPHEIGHT, false);
		SetEndPos(rand() % dfMAPWIDTH, rand() % dfMAPHEIGHT, false);


		// A*
		printf_s("A* start..\n");
		if (_astar.PathFind(_startPos, _endPos, _path) == false) {
			//경로찾기 실패
			printf_s("FAIL..\n");
			// 실패한 맵 찍기
			FILE *fp;
			fopen_s(&fp, "log/FAILMAP.log", "a");
			if (fp == NULL) {
				fopen_s(&fp, "log/FAILMAP.log", "w");
			}
			fseek(fp, 0, SEEK_END);
			fprintf_s(fp, "\n==============================================================\n");
			fprintf_s(fp, "A* (%d)\n",_iVcnt);
			for (int i = 0; i < dfMAPHEIGHT; i++) {
				for (int j = 0; j < dfMAPWIDTH; j++) {
					fprintf_s(fp, "%3hd", g_map[i][j]);
				}
				fprintf_s(fp, "\n");
			}
			fprintf_s(fp, "\n==============================================================\n");
			fclose(fp);
		} else {
			// 경로찾기 성공
			_correction.Correction(_path, _corPath);// 경로보정
			printf_s("CLEAR!\n");
		}


		// 점프포인트 서치
		printf_s("JPS start..\n");
		if (_jps.PathFind(_startPos, _endPos, _path) == false) {
			//경로찾기 실패
			printf_s("FAIL..\n");
			// 실패한 맵 찍기
			FILE *fp;
			fopen_s(&fp, "log/FAILMAP.log", "a");
			if (fp == NULL) {
				fopen_s(&fp, "log/FAILMAP.log", "w");
			}
			fseek(fp, 0, SEEK_END);
			fprintf_s(fp, "\n==============================================================\n");
			fprintf_s(fp, "JPS (%d)\n", _iVcnt);
			for (int i = 0; i < dfMAPHEIGHT; i++) {
				for (int j = 0; j < dfMAPWIDTH; j++) {
					fprintf_s(fp, "%3hd", g_map[i][j]);
				}
				fprintf_s(fp, "\n");
			}
			fprintf_s(fp, "\n==============================================================\n");
			fclose(fp);
		} else {
			// 경로찾기 성공
			_correction.Correction(_path, _corPath);// 경로보정
			printf_s("CLEAR!\n");
		}

		if (_iVcnt % 10 == 9) {
			// Alloc_YYYYMMDD_HHMMSS.log
			char filename[50];	// 저장할파일 이름

			// 시간구하기
			time_t now = time(0);
			struct tm tstruct;

			localtime_s(&tstruct, &now);
			strftime(filename, sizeof(filename), "log/Pathfinder_%Y%m%e_%H%M%S.log", &tstruct); // (Pathfinder_20210828_173840.log)
			PRO_PRINT(filename);
			g_memoryPool.Monitoring(filename);
		}

		printf_s("=======================================\n");

	}
}


void CFramework::GenerateMap() {
	_mapGenerator.Generate();
}

void CFramework::LoadMap() {
	FILE *fp;
	fopen_s(&fp, "map/map.txt", "r");
	if (fp == NULL) {

		return;
	}
	fprintf_s(fp, "JPS (%d)\n", _iVcnt);
	int map;
			memset(g_map, 0, sizeof(g_map));
	for (int i = 0; i < dfMAPHEIGHT; i++) {
		for (int j = 0; j < dfMAPWIDTH; j++) {
			fscanf_s(fp, "%d", &map);
			switch (map) {
			case 1:
				g_map[i][j] = (short) eMAP::Wall;
				break;
			case 2:
				SetStartPos(j, i, false);
				break;
			case 3:
				SetEndPos (j, i, false);
				break;
			default:
				break;
			}
		}
	}
	fclose(fp);
}

void CFramework::StartLine(int iX, int iY) {
	_bLine = true;
	_sX = iX;
	_sY = iY;
}

void CFramework::DrawLine(int iX, int iY) {
	if (_bLine == false) return;
	int sx = _sX / dfRectLen;
	int sy = _sY / dfRectLen;
	int ex = iX / dfRectLen;
	int ey = iY / dfRectLen;

	_eX = iX;
	_eY = iY;

	this->ResetMap();
	_correction.DrawLine(sx, sy, ex, ey);
}

void CFramework::ToggleVerif() {
	_bVerif = !_bVerif;
	if (_bVerif == true) {
		_iVcnt = 0;
	} 
}


struct stPATH *CFramework::GetPositionInfo(int x, int y) {
	//return _astar.GetPathNode(x, y);
	return _jps.GetPathNode(x, y);
}

