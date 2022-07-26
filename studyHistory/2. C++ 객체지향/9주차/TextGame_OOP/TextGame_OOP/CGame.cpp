#include "pch.h"
#include "CGame.h"
#include "Managers.h"

#include "IObject.h"

#include "CSpaghetti.h"
#include "CTurret.h"
#include "CMonster.h"
#include "CWall.h"
#include "CPortal.h"

// sceneFilename의 파일을 _scenefile로 불러들인후 파일내용 파싱
CGame::CGame(const WCHAR *sceneFilename) {
	// sceneFilename 파일 불러오기
	_scenefile = new CParser(sceneFilename); 

	// 이동에 사용할 틱 카운터 초기화
	_moveTick = 0;
	_attackTick = 0;

	// 이 스테이지 정보 파싱하기
#pragma region Parsing
	// 맵데이터
	_scenefile->SetNamespace(L"MAPDATA");

	_scenefile->TryGetValue(L"iMapH", _mapHigh);
	_scenefile->TryGetValue(L"iMapW", _mapWidt);

	_mapData = new WCHAR[_mapWidt * _mapHigh + _mapHigh]; // 넓이 + 개행문자
	if (_scenefile->TryGetValue(L"sMap", _mapData) == false) {
		CRASH();
	}


	// 다음스테이지 정보
	_scenefile->SetNamespace(L"STAGE");

	if (_scenefile->TryGetValue(L"sNextScene", _nextscene) == false) {
		CRASH();
	}


	// 플레이어 정보
	_scenefile->SetNamespace(L"PLAYER");

	_scenefile->TryGetValue(L"iPlayerHp", _playerHp);
	_scenefile->TryGetValue(L"iPlayerDamage", _playerDamage);
	_scenefile->TryGetValue(L"iPlayerAttackDelay", _playerAttackDelay);
	_scenefile->TryGetValue(L"iPlayerMoveDelay", _playerMoveDelay);


	// 몬스터 정보
	_scenefile->SetNamespace(L"MONSTER");

	// 스파게티
	_scenefile->TryGetValue(L"iSpaghettiHp", _spaghettiHp);
	_scenefile->TryGetValue(L"iSpaghettiDamage", _spaghettiDamage);
	_scenefile->TryGetValue(L"iSpaghettiAttackSpeed", _spaghettiAtspeed);
	_scenefile->TryGetValue(L"iSpaghettiMoveSpeed", _spaghettiMospeed);

	// 터렛
	_scenefile->TryGetValue(L"iTurretHp", _turretHp);
	_scenefile->TryGetValue(L"iTurretDamage", _turretDamage);
	_scenefile->TryGetValue(L"iTurretAttackSpeed", _turretAtspeed);
#pragma endregion

}


// 스테이지의 초기상태 (리셋을 할시 이상태로 돌아올 상태)
void CGame::Init() {
	// 프레임워크의 오브젝트 모두 비우기
	I_FRAMEWORK->claer();

	// _mapData의 데이터 대로 맵 생성 하기
	int i = 0;
	for (int y = 0; y < _mapHigh; y++) {
		for (int x = 0; x < _mapWidt; x++) {
			WCHAR buffer = *(_mapData + i); // (x,y) 에 있는 문자
			// 그 문자가 뭐냐면..
			switch (buffer) {
			case L'#':
				// 벽
				INSTANTIATE(new CWall(x, y));
				break;
			case L'P':
				// 플레이어
				_pPlayer = (CPlayer *) INSTANTIATE(new CPlayer(x, y, _playerHp, _playerDamage));
				break;
			case L'S':
				// 스파게티
				INSTANTIATE(new CSpaghetti(x, y, _spaghettiHp, _spaghettiDamage, _spaghettiAtspeed, _spaghettiMospeed));
				break;
			case L'T':
				// 터렛
				INSTANTIATE(new CTurret(x, y, _turretHp, _turretDamage, _turretAtspeed));
				break;
			case L'@':
				// 포탈
				INSTANTIATE(new CPortal(x, y, _nextscene));
				break;
			case L'M':
				// 테스트용 몬스터
				INSTANTIATE(new CMonster(x, y, 5, 5));
			}
			++i; // 다음 생성할것
		}
		++i; // 개행문자 스킵
	}

	
}

// 플레이어의 입력을 체크하여 큐에 넣음, 플레이어에 인풋 메시지 큐 전달
void CGame::Update() {
	// 인풋체크
	// 1. 혹시 모를 큐를 비우고
	// 2. 큐에 입력메시지를 넣고
	// 3. 플레이어에게 전달
	_input.clear();
	Input();
	_pPlayer->Input(_input);
}

// UI 출력
// TODO : UI따로만들기..
void CGame::Render() {
	// SCREENBUFFER 싱글톤 불러서
	CScreenBuffer *screenInst = I_SCREENBUFFER;
	// HP : 출력
	screenInst->Sprite_Draw(0, _mapHigh + 1, "HP : ", 5);
	// _pPlayer->_hp 만큼 o 출력
	for (int i = 0; i < _pPlayer->_hp; i++) {
		screenInst->Sprite_Draw(5 + i, _mapHigh + 1, 'o');
	}
}

// 이 씬이 파괴될때 _mapData, _scenefile 메모리할당 해지
void CGame::Destroy() {
	delete[] _mapData;
	delete _scenefile;
}

// Update()에서 메시지큐에 넣는 부분
void CGame::Input() {
	frame_t now = I_FRAMEWORK->Frame();
	
	// 이동
	// PLAYER_MOVE_DELAYms 마다 이동키 입력 가능
	if (now - _moveTick >= _playerMoveDelay) {
		// UP키가 눌려있는 상태
		if (GetAsyncKeyState(VK_UP) & 0x8000) {
			// 위로
			_input.push(InputMessage::Up);
		}
		// RIGHT키가 눌려있는 상태
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			// 오른쪽으로
			_input.push(InputMessage::Right);
		}
		// DOWN키가 눌려있는 상태
		if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
			// 아래로
			_input.push(InputMessage::Down);
		}
		// LEFT키가 눌려있는 상태
		if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			// 왼쪽으로
			_input.push(InputMessage::Left);
		}
		_moveTick = now; // 틱카운터 갱신
	}

	// 공격
	// PLAYER_ATTACK_DELAYms 마다 공격키 입력 가능
	if (now - _attackTick >= _playerAttackDelay) {
		// W 눌려져있으면
		if (GetAsyncKeyState('W') & 0x8000) {
			// 위로
			_input.push(InputMessage::AttackUp);
		}
		// D 눌려져있으면
		if (GetAsyncKeyState('D') & 0x8000) {
			// 오른쪽으로
			_input.push(InputMessage::AttackRight);
		}
		// S 눌려져있으면
		if (GetAsyncKeyState('S') & 0x8000) {
			// 아래로
			_input.push(InputMessage::AttackDown);
		}
		// A 눌려져있으면
		if (GetAsyncKeyState('A') & 0x8000) {
			// 왼쪽으로
			_input.push(InputMessage::AttackLeft);
		}
		_attackTick = now;// 틱카운터 갱신
	}
}
