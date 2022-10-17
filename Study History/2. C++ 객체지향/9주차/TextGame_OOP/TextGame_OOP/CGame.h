#pragma once
#include "IScene.h"
#include "CQueue.h"
#include "CPlayer.h"
#include "CParser.h"

// ============================================================================
//						CGame
// ---------------------------------------------------------------------------
// 상속
// IScene <- CGame
// ----
// 게임 진행할 오브젝트
// 스테이지 데이터 파일에 따라 게임이 달라짐
// ----
// _input	  : 인풋 메시지를 저장할 큐
// _pPlayer	  : 플레이어의 포인터를 캐싱
// _scenefile : 스테이지 파일을 파싱할 파서
// ----
// CGame(sceneFilename) : sceneFilename의 파일을 _scenefile로 불러들인후 파일내용 파싱
// Init()	: 스테이지의 초기상태 (리셋을 할시 이상태로 돌아올 상태)
// Update()	: 플레이어의 입력을 체크하여 큐에 넣음, 플레이어에 인풋 메시지 큐 전달
// Render()	: 플레이어의 현재 Hp출력
// Destroy(): 이 씬이 파괴될때 _mapData, _scenefile 메모리할당 해지
// Input()	: Update()에서 메시지큐에 넣는 부분
// ============================================================================

class CGame : public  IScene {
private :
	CQueue< InputMessage> _input;
	CPlayer *_pPlayer;
	
	// stagedata
	CParser *_scenefile;

	// -------------------------------------------
	// 그 스테이지 데이터
	// .dat 파일에서 파싱하여 해당 변수에 대입
	// -------------------------------------------

	// map
	int _mapHigh;			// 맵 높이
	int _mapWidt;			// 맵 너비
	WCHAR *_mapData;		// 맵이 어떻게 생겼는지 문자열
	WCHAR _nextscene[16];	// 다음 씬(포탈에 등록)

	// player
	frame_t _attackTick;	// 몇 프레임 전에 마지막으로 공격했는지
	frame_t _moveTick;		// 몇 프레임 전에 마지막으로 움직였는지
	int _playerHp;			// 플레이어 체력
	int _playerDamage;		// 플레이어 공격력
	int _playerAttackDelay;	// 몇 프레임 마다 공격할건지
	int _playerMoveDelay;	// 몇 프레임 마다 움직일것 인지?
		
	// CSpaghetti
	int _spaghettiHp;		// 스파게티 체력
	int _spaghettiDamage;	// 스파게티 공격력
	int _spaghettiAtspeed;	// 스파게티 공격속도
	int _spaghettiMospeed;	// 스파게티 이동속도

	// CTurret
	int _turretHp;			// 터렛 체력
	int _turretDamage;		// 터렛 공격력
	int _turretAtspeed;		// 터렛 공격속도

public:
	CGame(const WCHAR* sceneFilename);	// sceneFilename의 파일을 _scenefile로 불러들인후 파일내용 파싱
	virtual void Init();				// 스테이지의 초기상태 (리셋을 할시 이상태로 돌아올 상태)
	virtual void Update();				// 플레이어의 입력을 체크하여 큐에 넣음, 플레이어에 인풋 메시지 큐 전달
	virtual void Render();				// 플레이어의 현재 Hp출력
	virtual void Destroy();				// 이 씬이 파괴될때 _mapData, _scenefile 메모리할당 해지
	void Input();						// Update()에서 메시지큐에 넣는 부분
};

