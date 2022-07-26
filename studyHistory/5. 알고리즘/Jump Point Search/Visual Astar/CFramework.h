#pragma once
#include "Global.h"
#include "DrawMap.h"
#include "CList.h"
#include "CAstar.h"
#include "CMapGenerator.h"
#include "CJumpPoint.h"
#include "CCorrection.h"
class CFramework {
#pragma region Singleton 
	// 싱글톤
private:
	CFramework();	
	virtual ~CFramework();
public:
	// 전역 인스턴스를 얻어올 전역 함수
	static CFramework *GetInstance() {
		static CFramework _Instance;
		return &_Instance;
	};
#pragma endregion

private:
	// 플래그
	bool _bClick;
	bool _bWallCreate; // 벽이 생성야하는지 (true : 생성, false : 파괴)
	// 클래스
	CDisplayMap _displayMap; // 맵 그리기 클래스
	CAstar _astar;//astar
	CJumpPoint _jps; // jump point
	CMapGenerator _mapGenerator;// 미로(장애물) 만들기
	CCorrection _correction; // 보정


	// 시작 끝 지잠
	stPOS _startPos;
	stPOS _endPos;


	// 윈도우 정보 과정 보여주기 위해!
	HWND _hWnd;
	HPEN _pathPen;
	HPEN _shortPathPen;


	// 찾은 경로
	CList<stPOS> _path;
	CList<stPOS> _corPath; // 보정경로

	// TEMP 라인 긋기
	bool _bLine;
	int _sX;
	int _sY;
	int _eX;
	int _eY;


	// 자동 검증
	bool _bVerif;
	int _iVcnt;

public:
	void Init(HWND);		// 초기화
	void Display();// 보여주기

	void ResetMap(); // 맵 리셋
	void ClearMap();// 벽, 시작,끝지점은 가만히두고 open/close를 empty로
	void ClearData();// 맵은 가만히 두고 이전에 실행한 데이터 모두 지우기;
	void ToggleWall(int x, int y);// 픽셀로 좌표 찾아서 벽생성
	void SetStartPos(int x, int y, bool bWorld = true);// 시작지점 지정
	void SetEndPos(int x, int y, bool bWorld = true);// 끝지점 지정

	void LButtonDown(int x, int y); // 마우스 왼클릭 누르면 벽생성 시작
	void LButtonUp();// 마우스 왼클릭 때면 벽생성 끝

	void FindPath(); // Astar실행
	void Running(); // astart보여주기


	void GenerateMap();// 맵생성
	void LoadMap();
	void StartLine(int iX, int iY);// 직선 시작점
	void DrawLine(int iX, int iY); // 직선 긋기

	void ToggleVerif();


	struct stPATH *GetPositionInfo(int x, int y);
};

