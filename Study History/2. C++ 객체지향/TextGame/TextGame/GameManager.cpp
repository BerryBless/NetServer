#include "pch.h"
#include "Player.h"
#include "Monster.h"
#include "Projectile.h"
#include "GameManager.h"
#include "DataManager.h"
#include "SceneManager.h"
int _stageHeight;										// 현 스테이지의 높이 Y(i)
int _stageWidth;										// 현 스테이지의 너비 X(j)
ObjectType _gameState[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];	// 현 스테이지의 "현재" 상태

#pragma region Management
// ============================================================================
//						게임을 처음 실행했을 초기화!
// ---------------------------------------------------------------------------
// 1. 콘솔 제어를 위한 준비작업					cs_Initial()
// 2. 콘솔 버퍼를 초기화 하는 작업				Buffer_Clear()
// 3. 파일에서 데이터를 받을준비를 하는 작업		Data_Init()
// ============================================================================
void Game_Init() {
	cs_Initial();
	Buffer_Clear();
	Data_Init();
}

// ============================================================================
//							게임 스테이지를 생성!
// ---------------------------------------------------------------------------
// 1. LoadObjectType()로 stageNum 번째 맵 데이터를 파일에서 불러온다.
// 2. 맵의 너이와 높비를 파싱한다.
// 3. 높이와 너비를 순회하며 하나씩 DATA SHEET 정수를 파싱한다
// 3-1. 플레이어 데이터를 만나면 해당 자리에 플레이어를 스폰합니다.
// 3-2. 몬스터 데이터를 만나면 해당 자리에 몬스터를 스폰합니다.
// ============================================================================
void Game_MakeStage(int stageNum) {
	// 멥데이터 파일 불러오기
	// 이미 버퍼에 데이터가 올라가 있으면 파일 부르지 않기
	if (stageNum != _stageNow) {
		Data_LoadStage(stageNum);
		_stageNow = stageNum;
	}

	// 스테이지 정보 초기화
	Projectile_Init();
	Monster_Init();
	memset(_gameState, 0, sizeof(int)*dfSCREEN_HEIGHT * dfSCREEN_WIDTH); // 스테이지 현재정보를 모두 0 으로 밀어줌

	int index = 0;	// 파싱할곳 인덱스
	int buffer;		// 스테이지 데이터를 입력받아두는 버퍼
	// 너비와 높이를 구한다!
	if (Parsing((char*)_stageData, &_stageWidth, index) == false) { CRASH(); }
	if (Parsing((char*)_stageData, &_stageHeight, index) == false) { CRASH(); }


	// 파싱
	for (int i = 0; i < _stageHeight; i++) {
		for (int j = 0; j < _stageWidth; j++) {

			if (Parsing((char*)_stageData, &buffer, index) == false) { CRASH(); }
			// 버퍼가 플레이어 타입이면
			if ((ObjectType)buffer == ObjectType::Player) {
				// 플레이어 리스폰
				Player_Spawn(j, i);
			}
			// 버퍼가 몬스터 범위 내라면
			else if (IS_MONSTER((ObjectType)buffer)) {
				Monster_Spawn(j, i, (ObjectType)buffer);
			}
			else {
				// TODO 잘못된 데이터 걸러내기
				_gameState[i][j] = (ObjectType)buffer;
			}
		}
	}
}
#pragma endregion

#pragma region GamePlay

// ============================================================================
//							_gameState를 버퍼에 그리기
// ---------------------------------------------------------------------------
// _gameState를 전체 순회하며 화면에 출력할 버퍼에 하나씩 넣어줍니다.
// ' ' - 0 : 빈칸
// ---------------------------------------------------------------------------
// 1~9 : 맵오브젝트 관련
// '#' - 1 : 벽 (못감)
// 'O' - 2 : 포탈 (다음 스테이지로!)
// 
// ---------------------------------------------------------------------------
// 10 ~ 19 게임플레이 관련
// 'P' - 10 : player
// 11 : 투사체
// 투사체가 가지는 방향에따라 위'^' 오른쪽'>' 아래'v' 왼쪽'<' 
// 
// ---------------------------------------------------------------------------
// 20 ~ 몬스터 관련
// 'M' - 20 : 패턴없는 몬스터  
// 'R' - 21 : 돌맹이			(4방향 랜덤으로 구름)
// 'T' - 22 : 터렛			(직선상에 플레이어가 있음 투사체 발싸)
// 'q' - 23 : 도플갱어		(플레이어의 반대방향으로 이동)
// ============================================================================
void Game_DrawStage() {
	// 스크린 버퍼에 스프라이트 출력
	for (int i = 0; i < _stageHeight; i++) {
		for (int j = 0; j < _stageWidth; j++) {
			switch (_gameState[i][j]) {
			case ObjectType::Blocked:	// 벽
				Sprite_Draw(j, i, '#');
				break;
			case ObjectType::Portal:	// 포탈
				Sprite_Draw(j, i, 'O');
				break;
			case ObjectType::Player:	// 플레이어
				Sprite_Draw(j, i, 'P');
				break;
			case ObjectType::Projectile: // 화살
				Projectile_Draw(j, i);	// 화살은 방향에 따라 다름으로 따로 호출합니다.
				break;
			case ObjectType::Monster:	// 테스트용 몬스터
				Sprite_Draw(j, i, 'M');
				break;
			case ObjectType::RockNRoll:	// 돌맹이
				Sprite_Draw(j, i, 'R');
				break;
			case ObjectType::Turret:	// 터렛
				Sprite_Draw(j, i, 'T');
				break;
			case ObjectType::Doppelganger: // 도플갱어
				Sprite_Draw(j, i, 'q');
				break;
			default:
				break;
			}
		}
	}
	Game_DrawUi();
}

// ============================================================================
//							버퍼에 UI그리기
// ---------------------------------------------------------------------------
// player의 hp만큼 @를 출력합니다
// HP : @@@
// ============================================================================
void Game_DrawUi() {
	Sprite_Draw(0, _stageHeight, "HP : ", 5);
	for (int i = 0; i < _player.Hp; i++) {
		Sprite_Draw(5 + i, _stageHeight, '@');
	}

}

// ============================================================================
//							 게임 로직
// ---------------------------------------------------------------------------
// // 무한루프 안의 게임로직을 실행합니다
// SceneType::Title
//		엔터키 입력을 기다립니다.
// SceneType::Load
//		스테이지를 로딩합니다
// SceneType::InGame
//		게임중에 돌아갈 업데이트를 합니다!
//		1. 투사체 업데이트
//		2. 플레이어가 업데이트
//		3. 몬스터 업데이트
//		4. 버퍼에 그림그리기
// SceneType::Ending
//		엔딩씬을 그리며 게임을 종료합니다.
//		이때 false를 반환 합니다!
// ============================================================================
bool Game_Logic() {
	switch (_sceneNow)	{
	case SceneType::Title:
		// 씬 출력
		Sprite_Draw(17, 3, "T E X T  G A M E",16);
		Sprite_Draw(22, 5, "P -> O",6);
		Sprite_Draw(3, 8, "MOVE : ARRROW KEY              ATTACK : A KEY",45);
		Sprite_Draw(18, 12, ">> PRESS ENTER <<",17);
		// 엔터키를 기다립니다.
		if (GetAsyncKeyState(VK_RETURN) & 0x8001) {
			// 엔터키가 눌렸으니 스테이지 로딩하라고 알려줍니다
			Scene_Load(SceneType::Load);
		}
		break;
	case SceneType::Load:
		PRO_BEGIN(L"LOAD"); // 시간측정 시작
		// _sceneStageNum 스테이지를 로딩합니다!
		Game_MakeStage(_sceneStageNum);
		Scene_Load(SceneType::InGame);
		PRO_END(L"LOAD");	// 시간측정 끝
		break;
	case SceneType::InGame:
		// 게임플레이중 게임이 업데이트 되는곳입니다.
		// 1. 투사체 업데이트
		// 2. 플레이어 업데이트
		// 3. 몬스터 업데이트
		// 4. 버퍼에 그림그리기
		PRO_BEGIN(L"IN GAME");// 시간측정 시작

		Projectile_Update();
		Player_Update();
		Monster_Update();
		Game_DrawStage();
		PRO_END(L"IN GAME");// 시간측정 끝
		break;
	case SceneType::Ending:
		// 엔딩씬
		// 스크린 버퍼를 지움
		Sprite_Draw(15, 9, "Y O U  W O N ! !",16);
		//게임종료!
		PRO_PRINT((const CHAR*)"GAME.log");
		return false;
	default:
		break;
	}
	return true;
}

// ============================================================================
//							좌표에 있는 오브젝트
// ---------------------------------------------------------------------------
// 해당좌표의 ObjectType 값을 반환합니다.
// 이를 이용하여 충돌처리를 합니다.
// 예를들면 ObjectType::Empty = 갈 수 있는 곳 입니다.
// ============================================================================
ObjectType Game_FindObject(int iX, int iY) {
	// 스테이지크기 범위를 벗어나면 그냥 막힌 벽취급(못감!)
	if (iY < 0 || iY >= _stageHeight ||
		iX < 0 || iX >= _stageWidth)
		return ObjectType::Blocked;
	// 그 위치에 있는 오브젝트 반환
	return _gameState[iY][iX];
}
// Position 구조체 에서도 쓸 수 있게 래핑
ObjectType Game_FindObject(Position pos) {
	return Game_FindObject(pos.X, pos.Y);
}

// ============================================================================
//							오브젝트 이동
// ---------------------------------------------------------------------------
// start에서 dest로 start에 있는 오브젝트를 이동시킵니다.
// 빈곳 ( Game_FindObject(dest) == ObjectType::Empty ) 
// 에서만 이동이 가능합니다.
// 빈곳이 아니라면 false, 이동했다면 true 를 반환합니다.
// ============================================================================
bool Game_MoveObject(Position start, Position dest) {
	// 목적지가 빈곳일때만 이동가능!
	if (Game_FindObject(dest) != ObjectType::Empty)
		return false;
	// 이동하고 원래자리 비워주기
	_gameState[dest.Y][dest.X] = _gameState[start.Y][start.X];
	_gameState[start.Y][start.X] = ObjectType::Empty;
	return true;
}

// ============================================================================
//							오브젝트 생성
// ---------------------------------------------------------------------------
// pos위치를 _gameState 에서 확인하고 빈곳이면 type의 오브젝트를 등록합니다!
// _gameState[pos] = type
// ============================================================================
bool Game_CreateObject(Position pos, ObjectType type) {
	bool hit = false; // 생성 되었냐?
	ObjectType there = ObjectType::Blocked; // 그곳에 있는 오브젝트
	there = Game_FindObject(pos);		// 그곳에 무언가있나?
	if (there == ObjectType::Empty) {
		// 비어있음으로 생성 가능!
		hit = true;
		_gameState[pos.Y][pos.X] = type;
	}
	return hit;
}
// ============================================================================
//							오브젝트 소멸
// ---------------------------------------------------------------------------
// _gameState[pos]의 데이터를 Empty로 바꿉니다!
// (pos)로도 사용할 수 있게 래핑해줍니다.
// ============================================================================
void Game_DissObject(Position pos) {
	Game_DissObject(pos.X, pos.Y);
}
void Game_DissObject(int iX, int iY) {
	_gameState[iY][iX] = ObjectType::Empty;
}

#pragma endregion